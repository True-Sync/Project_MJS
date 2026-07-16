#include "Cinematic/CinematicDirectorSubsystem.h"

#include "Cinematic/CinematicInputLockSubsystem.h"
#include "Cinematic/Director/CinematicSequenceDirectorService.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogCinematicSystem);

bool UCinematicDirectorSubsystem::PlayCinematic(const FCinematicPlaybackRequest& Request)
{
	if (bIsFinishing)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic rejected: a cinematic is currently finishing."));
		return false;
	}

	if (!Request.Sequence)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic failed: Sequence is missing."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic failed: World is missing."));
		return false;
	}

	if (!ShouldAllowPlaybackForNetworkPolicy(Request))
	{
		UE_LOG(LogCinematicSystem, Verbose, TEXT("PlayCinematic skipped by network policy. Sequence=%s Policy=%d"), *GetNameSafe(Request.Sequence), static_cast<int32>(Request.NetworkPolicy));
		return false;
	}

	if (ActiveSequencePlayer)
	{
		if (!Request.bStopPreviousCinematic)
		{
			UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic rejected: another cinematic is already active."));
			return false;
		}

		FinishCinematic(true);
	}

	ALevelSequenceActor* CreatedSequenceActor = nullptr;
	ActiveSequencePlayer = FCinematicSequenceDirectorService::CreateSequencePlayer(World, Request.Sequence, CreatedSequenceActor);
	ActiveSequenceActor = CreatedSequenceActor;
	if (!ActiveSequencePlayer)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic failed: could not create LevelSequencePlayer. Sequence=%s"), *GetNameSafe(Request.Sequence));
		return false;
	}

	TWeakObjectPtr<ULevelSequencePlayer> CreatedSequencePlayer = ActiveSequencePlayer;
	ActivePlayerController = ResolvePlayerController(Request);
	PreviousViewTarget = ActivePlayerController ? ActivePlayerController->GetViewTarget() : nullptr;
	ActiveBlendOutTime = FMath::Max(0.0f, Request.BlendOutTime);
	bShouldRestoreViewTarget = Request.bRestoreViewTarget;

	FTransform AnchorWorldTransform = FTransform::Identity;
	bool bAppliedDynamicTransform = false;
	FCinematicSequenceDirectorService::ConfigureSequenceActor(
		ActiveSequenceActor,
		ActivePlayerController,
		Request,
		AnchorWorldTransform,
		bAppliedDynamicTransform);

	BuildActiveContext(Request, AnchorWorldTransform, bAppliedDynamicTransform);
	ParticipantCoordinator.CollectParticipants(World, Request);
	PostActionExecutor.BeginPlayback(
		World,
		Request.PostAction,
		FTimerDelegate::CreateUObject(this, &UCinematicDirectorSubsystem::ExecutePlaybackLevelLoad));
	ParticipantCoordinator.NotifyParticipantsStarted(ActiveContext);

	if (ActiveSequencePlayer != CreatedSequencePlayer.Get())
	{
		UE_LOG(LogCinematicSystem, Verbose,
			TEXT("PlayCinematic aborted during participant startup. Sequence=%s"),
			*GetNameSafe(Request.Sequence));
		return false;
	}

	ActiveSequencePlayer->OnFinished.AddDynamic(this, &UCinematicDirectorSubsystem::HandleSequenceFinished);
	ActiveSequencePlayer->Play();

	UE_LOG(LogCinematicSystem, Log, TEXT("PlayCinematic succeeded: Sequence=%s Participants=%d"), *GetNameSafe(Request.Sequence), ParticipantCoordinator.Num());
	return true;
}

void UCinematicDirectorSubsystem::StopCinematic()
{
	FinishCinematic(true);
}

bool UCinematicDirectorSubsystem::IsCinematicPlaying() const
{
	return ActiveSequencePlayer && ActiveSequencePlayer->IsPlaying();
}

void UCinematicDirectorSubsystem::HandleSequenceFinished()
{
	FinishCinematic(false);
}

void UCinematicDirectorSubsystem::FinishCinematic(bool bStopPlayback)
{
	if (bIsFinishing)
	{
		return;
	}

	if (!ActiveSequencePlayer && !ActiveSequenceActor)
	{
		if (bStopPlayback)
		{
			PostActionExecutor.Cancel(GetWorld());
		}

		return;
	}

	bIsFinishing = true;
	const FCinematicPostActionConfig CompletedPostAction = PostActionExecutor.GetActiveConfig();
	PostActionExecutor.Cancel(GetWorld());

	if (ActiveSequencePlayer)
	{
		ActiveSequencePlayer->OnFinished.RemoveDynamic(this, &UCinematicDirectorSubsystem::HandleSequenceFinished);

		if (bStopPlayback && ActiveSequencePlayer->IsPlaying())
		{
			ActiveSequencePlayer->Stop();
		}
	}

	const bool bNaturalFinish = !bStopPlayback;
	TWeakObjectPtr<APawn> PawnToPreserve;
	FTransform CinematicEndTransform = FTransform::Identity;

	if (bNaturalFinish && ActivePlayerController)
	{
		if (APawn* Pawn = ActivePlayerController->GetPawn())
		{
			if (IsValid(Pawn))
			{
				PawnToPreserve = Pawn;
				CinematicEndTransform = Pawn->GetActorTransform();
			}
		}
	}

	ParticipantCoordinator.NotifyParticipantsEnded(ActiveContext);

	if (PawnToPreserve.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([PawnToPreserve, CinematicEndTransform]()
			{
				if (APawn* Pawn = PawnToPreserve.Get())
				{
					Pawn->TeleportTo(CinematicEndTransform.GetLocation(), CinematicEndTransform.Rotator(), false, true);
				}
			});
		}
	}

	RestoreViewTarget();
	FCinematicSequenceDirectorService::DestroySequenceActor(ActiveSequenceActor);

	ActiveSequencePlayer = nullptr;
	ActiveSequenceActor = nullptr;
	ActivePlayerController = nullptr;
	PreviousViewTarget = nullptr;
	ParticipantCoordinator.Reset();
	ActiveContext = FCinematicPlaybackContext();
	ActiveBlendOutTime = 0.15f;
	bShouldRestoreViewTarget = true;

	if (bNaturalFinish)
	{
		PostActionExecutor.ExecuteOnNaturalFinish(
			GetWorld(),
			CompletedPostAction,
			FTimerDelegate::CreateUObject(this, &UCinematicDirectorSubsystem::ExecutePendingPostAction));
	}

	bIsFinishing = false;
}

void UCinematicDirectorSubsystem::RestoreViewTarget()
{
	if (!bShouldRestoreViewTarget || !ActivePlayerController)
	{
		return;
	}

	AActor* TargetToRestore = IsValid(PreviousViewTarget) ? PreviousViewTarget.Get() : nullptr;
	if (!TargetToRestore)
	{
		TargetToRestore = ActivePlayerController->GetPawn();
	}

	if (IsValid(TargetToRestore))
	{
		ActivePlayerController->SetViewTargetWithBlend(TargetToRestore, ActiveBlendOutTime);
	}
}

bool UCinematicDirectorSubsystem::ShouldAllowPlaybackForNetworkPolicy(const FCinematicPlaybackRequest& Request) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	switch (Request.NetworkPolicy)
	{
	case ECinematicNetworkPolicy::LocalOnly:
		return !World->IsNetMode(NM_DedicatedServer);
	case ECinematicNetworkPolicy::AuthorityOnly:
		return !World->IsNetMode(NM_Client);
	case ECinematicNetworkPolicy::AnyNetMode:
	default:
		return true;
	}
}

APlayerController* UCinematicDirectorSubsystem::ResolvePlayerController(const FCinematicPlaybackRequest& Request) const
{
	if (Request.PlayerController)
	{
		return Request.PlayerController;
	}

	if (const APawn* InstigatorPawn = Cast<APawn>(Request.InstigatorActor))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(InstigatorPawn->GetController()))
		{
			return PlayerController;
		}
	}

	if (const APawn* SubjectPawn = Cast<APawn>(Request.SubjectActor))
	{
		if (APlayerController* PlayerController = Cast<APlayerController>(SubjectPawn->GetController()))
		{
			return PlayerController;
		}
	}

	UWorld* World = GetWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}

void UCinematicDirectorSubsystem::BuildActiveContext(const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform, bool bAppliedDynamicTransform)
{
	ActiveContext.Sequence = Request.Sequence;
	ActiveContext.InstigatorActor = Request.InstigatorActor;
	ActiveContext.SubjectActor = Request.SubjectActor;
	ActiveContext.PlayerController = ActivePlayerController;
	ActiveContext.SequenceActor = ActiveSequenceActor;
	ActiveContext.ParticipantScope = Request.ParticipantScope;
	ActiveContext.AnchorMode = Request.AnchorMode;
	ActiveContext.RotationSource = Request.RotationSource;
	ActiveContext.AnchorWorldTransform = AnchorWorldTransform;
	ActiveContext.bAppliedDynamicTransform = bAppliedDynamicTransform;
}

FString UCinematicDirectorSubsystem::GetCinematicStatusSummary() const
{
	if (!IsCinematicPlaying())
	{
		return TEXT("Cinematic: Not Playing");
	}

	TArray<FString> Lines;
	Lines.Add(TEXT("Cinematic: Playing"));
	Lines.Add(FString::Printf(TEXT("  Sequence: %s"), *GetNameSafe(ActiveContext.Sequence)));
	Lines.Add(FString::Printf(TEXT("  AnchorMode: %d"), static_cast<int32>(ActiveContext.AnchorMode)));
	Lines.Add(FString::Printf(TEXT("  RotationSource: %d"), static_cast<int32>(ActiveContext.RotationSource)));
	Lines.Add(FString::Printf(TEXT("  Participants: %d"), ParticipantCoordinator.Num()));

	const UCinematicInputLockSubsystem* InputLockSubsystem = GetWorld() ? GetWorld()->GetSubsystem<UCinematicInputLockSubsystem>() : nullptr;
	const bool bInputLocked = InputLockSubsystem && IsValid(ActivePlayerController) &&
		(InputLockSubsystem->IsMoveInputLocked(ActivePlayerController) ||
			InputLockSubsystem->IsLookInputLocked(ActivePlayerController) ||
			InputLockSubsystem->IsGameplayInputLocked(ActivePlayerController));
	Lines.Add(FString::Printf(TEXT("  InputLocked: %s"), bInputLocked ? TEXT("Yes") : TEXT("No")));

	return FString::Join(Lines, TEXT("\n"));
}

void UCinematicDirectorSubsystem::Deinitialize()
{
	FinishCinematic(true);
	PostActionExecutor.Cancel(GetWorld());
	Super::Deinitialize();
}

void UCinematicDirectorSubsystem::ExecutePlaybackLevelLoad()
{
	FCinematicPostActionConfig Config;
	if (!PostActionExecutor.ConsumeDuringPlayback(GetWorld(), Config))
	{
		return;
	}

	if (Config.bStopCinematicBeforeLoad)
	{
		FinishCinematic(true);
	}

	FCinematicPostActionExecutor::ExecuteLevelLoad(GetWorld(), Config, TEXT("DuringPlayback"));
}

void UCinematicDirectorSubsystem::ExecutePendingPostAction()
{
	FCinematicPostActionConfig Config;
	if (PostActionExecutor.ConsumePendingPostAction(GetWorld(), Config))
	{
		FCinematicPostActionExecutor::ExecuteLevelLoad(GetWorld(), Config, TEXT("OnFinishDelayed"));
	}
}
