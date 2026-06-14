#include "Cinematic/CinematicDirectorSubsystem.h"

#include "Camera/PlayerCameraManager.h"
#include "Cinematic/CinematicParticipant.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlaybackSettings.h"

bool UCinematicDirectorSubsystem::PlayCinematic(const FCinematicPlaybackRequest& Request)
{
	if (!Request.Sequence)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCinematic failed: Sequence is missing."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCinematic failed: World is missing."));
		return false;
	}

	if (!ShouldAllowPlaybackForNetworkPolicy(Request))
	{
		UE_LOG(LogTemp, Verbose, TEXT("PlayCinematic skipped by network policy. Sequence=%s Policy=%d"), *GetNameSafe(Request.Sequence), static_cast<int32>(Request.NetworkPolicy));
		return false;
	}

	if (ActiveSequencePlayer)
	{
		if (!Request.bStopPreviousCinematic)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayCinematic rejected: another cinematic is already active."));
			return false;
		}

		FinishCinematic(true);
	}

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ALevelSequenceActor* CreatedSequenceActor = nullptr;
	ActiveSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(World, Request.Sequence, PlaybackSettings, CreatedSequenceActor);
	ActiveSequenceActor = CreatedSequenceActor;
	if (!ActiveSequencePlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCinematic failed: could not create LevelSequencePlayer. Sequence=%s"), *GetNameSafe(Request.Sequence));
		ActiveSequenceActor = nullptr;
		return false;
	}

	ActivePlayerController = ResolvePlayerController(Request);
	PreviousViewTarget = ActivePlayerController ? ActivePlayerController->GetViewTarget() : nullptr;
	ActiveBlendOutTime = FMath::Max(0.0f, Request.BlendOutTime);
	bShouldRestoreViewTarget = Request.bRestoreViewTarget;

	FTransform AnchorWorldTransform = FTransform::Identity;
	bool bAppliedDynamicTransform = false;
	ApplyDynamicTransform(Request, AnchorWorldTransform, bAppliedDynamicTransform);
	ApplyBindingOverrides(Request);
	DrawDebugAnchorTransform(Request, bAppliedDynamicTransform ? AnchorWorldTransform : (ActiveSequenceActor ? ActiveSequenceActor->GetActorTransform() : FTransform::Identity));

	BuildActiveContext(Request, AnchorWorldTransform, bAppliedDynamicTransform);
	CollectParticipants(Request);
	NotifyParticipantsStarted();

	ActiveSequencePlayer->OnFinished.AddDynamic(this, &UCinematicDirectorSubsystem::HandleSequenceFinished);
	ActiveSequencePlayer->Play();

	UE_LOG(LogTemp, Log, TEXT("PlayCinematic succeeded: Sequence=%s Participants=%d"), *GetNameSafe(Request.Sequence), ActiveParticipants.Num());
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
	if (!ActiveSequencePlayer && !ActiveSequenceActor)
	{
		return;
	}

	if (ActiveSequencePlayer)
	{
		ActiveSequencePlayer->OnFinished.RemoveDynamic(this, &UCinematicDirectorSubsystem::HandleSequenceFinished);

		if (bStopPlayback && ActiveSequencePlayer->IsPlaying())
		{
			ActiveSequencePlayer->Stop();
		}
	}

	NotifyParticipantsEnded();
	RestoreViewTarget();

	if (IsValid(ActiveSequenceActor))
	{
		ActiveSequenceActor->Destroy();
	}

	ActiveSequencePlayer = nullptr;
	ActiveSequenceActor = nullptr;
	ActivePlayerController = nullptr;
	PreviousViewTarget = nullptr;
	ActiveParticipants.Reset();
	ActiveContext = FCinematicPlaybackContext();
	ActiveBlendOutTime = 0.15f;
	bShouldRestoreViewTarget = true;
}

void UCinematicDirectorSubsystem::RestoreViewTarget()
{
	if (bShouldRestoreViewTarget && ActivePlayerController && PreviousViewTarget)
	{
		ActivePlayerController->SetViewTargetWithBlend(PreviousViewTarget, ActiveBlendOutTime);
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
	ActiveContext.bAffectAllParticipants = Request.bAffectAllParticipants;
	ActiveContext.AnchorWorldTransform = AnchorWorldTransform;
	ActiveContext.bAppliedDynamicTransform = bAppliedDynamicTransform;
}

void UCinematicDirectorSubsystem::ApplyBindingOverrides(const FCinematicPlaybackRequest& Request) const
{
	if (!ActiveSequenceActor)
	{
		return;
	}

	for (const FCinematicBindingOverride& BindingOverride : Request.BindingOverrides)
	{
		if (BindingOverride.BindingTag.IsNone())
		{
			continue;
		}

		TArray<AActor*> BoundActors;
		for (AActor* Actor : BindingOverride.Actors)
		{
			if (IsValid(Actor))
			{
				BoundActors.Add(Actor);
			}
		}

		if (BoundActors.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Cinematic binding override skipped: Tag=%s has no valid actors."), *BindingOverride.BindingTag.ToString());
			continue;
		}

		ActiveSequenceActor->SetBindingByTag(BindingOverride.BindingTag, BoundActors, BindingOverride.bAllowBindingsFromAsset);
	}
}

void UCinematicDirectorSubsystem::ApplyDynamicTransform(const FCinematicPlaybackRequest& Request, FTransform& OutAnchorWorldTransform, bool& bOutAppliedDynamicTransform) const
{
	OutAnchorWorldTransform = FTransform::Identity;
	bOutAppliedDynamicTransform = false;

	if (!ActiveSequenceActor || Request.AnchorMode == ECinematicAnchorMode::AuthoredWorld)
	{
		return;
	}

	OutAnchorWorldTransform = ResolveDynamicAnchorTransform(Request);
	ActiveSequenceActor->SetActorTransform(OutAnchorWorldTransform);

	UDefaultLevelSequenceInstanceData* InstanceData = NewObject<UDefaultLevelSequenceInstanceData>(ActiveSequenceActor, NAME_None, RF_Transient);
	if (InstanceData)
	{
		InstanceData->TransformOrigin = OutAnchorWorldTransform;
		ActiveSequenceActor->DefaultInstanceData = InstanceData;
		ActiveSequenceActor->bOverrideInstanceData = true;
	}

	bOutAppliedDynamicTransform = true;
}

FTransform UCinematicDirectorSubsystem::ResolveDynamicAnchorTransform(const FCinematicPlaybackRequest& Request) const
{
	FTransform AnchorTransform = FTransform::Identity;

	if (Request.AnchorMode == ECinematicAnchorMode::ExplicitTransform)
	{
		AnchorTransform = Request.ExplicitWorldTransform;
	}
	else
	{
		const AActor* AnchorActor = ResolveAnchorActor(Request);
		AnchorTransform = ResolveActorOrSocketTransform(AnchorActor, Request.AnchorSocketName);

		if (Request.AnchorMode == ECinematicAnchorMode::InstigatorToSubject)
		{
			const FTransform TargetTransform = ResolveActorOrSocketTransform(Request.SubjectActor, Request.TargetSocketName);
			const FVector DirectionToTarget = TargetTransform.GetLocation() - AnchorTransform.GetLocation();
			if (!DirectionToTarget.IsNearlyZero())
			{
				const FRotator LookAtRotation = FRotationMatrix::MakeFromX(DirectionToTarget.GetSafeNormal()).Rotator();
				AnchorTransform.SetRotation(NormalizeCinematicRotation(LookAtRotation, Request.bUseYawOnly).Quaternion());
			}
		}
	}

	const FRotator FinalRotation = ResolveRotation(Request, AnchorTransform);
	AnchorTransform.SetRotation(FinalRotation.Quaternion());
	AnchorTransform.NormalizeRotation();

	FTransform FinalTransform = Request.RelativeTransform * AnchorTransform;
	FinalTransform.NormalizeRotation();
	return FinalTransform;
}

FTransform UCinematicDirectorSubsystem::ResolveActorOrSocketTransform(const AActor* Actor, FName SocketName) const
{
	if (!IsValid(Actor))
	{
		return FTransform::Identity;
	}

	if (!SocketName.IsNone())
	{
		TArray<USceneComponent*> SceneComponents;
		Actor->GetComponents(SceneComponents);

		for (const USceneComponent* SceneComponent : SceneComponents)
		{
			if (IsValid(SceneComponent) && SceneComponent->DoesSocketExist(SocketName))
			{
				return SceneComponent->GetSocketTransform(SocketName, RTS_World);
			}
		}
	}

	return Actor->GetActorTransform();
}

AActor* UCinematicDirectorSubsystem::ResolveAnchorActor(const FCinematicPlaybackRequest& Request) const
{
	if (Request.AnchorActor)
	{
		return Request.AnchorActor;
	}

	switch (Request.AnchorMode)
	{
	case ECinematicAnchorMode::SubjectActor:
		return Request.SubjectActor;
	case ECinematicAnchorMode::InstigatorActor:
	case ECinematicAnchorMode::InstigatorToSubject:
		return Request.InstigatorActor;
	default:
		return nullptr;
	}
}

FRotator UCinematicDirectorSubsystem::ResolveRotation(const FCinematicPlaybackRequest& Request, const FTransform& AnchorTransform) const
{
	FRotator Rotation = AnchorTransform.Rotator();

	switch (Request.RotationSource)
	{
	case ECinematicRotationSource::InstigatorActor:
		Rotation = ResolveActorOrSocketTransform(Request.InstigatorActor, Request.AnchorSocketName).Rotator();
		break;
	case ECinematicRotationSource::SubjectActor:
		Rotation = ResolveActorOrSocketTransform(Request.SubjectActor, Request.TargetSocketName).Rotator();
		break;
	case ECinematicRotationSource::PlayerControlRotation:
		if (ActivePlayerController)
		{
			Rotation = ActivePlayerController->GetControlRotation();
		}
		break;
	case ECinematicRotationSource::PlayerCameraRotation:
		if (ActivePlayerController && ActivePlayerController->PlayerCameraManager)
		{
			Rotation = ActivePlayerController->PlayerCameraManager->GetCameraRotation();
		}
		else if (ActivePlayerController)
		{
			Rotation = ActivePlayerController->GetControlRotation();
		}
		break;
	case ECinematicRotationSource::ExplicitRotation:
		Rotation = Request.ExplicitRotation;
		break;
	case ECinematicRotationSource::AnchorTransform:
	default:
		break;
	}

	return NormalizeCinematicRotation(Rotation, Request.bUseYawOnly);
}

FRotator UCinematicDirectorSubsystem::NormalizeCinematicRotation(const FRotator& Rotation, bool bUseYawOnly) const
{
	FRotator NormalizedRotation = Rotation;
	NormalizedRotation.Normalize();

	if (bUseYawOnly)
	{
		NormalizedRotation.Pitch = 0.0f;
		NormalizedRotation.Roll = 0.0f;
		NormalizedRotation.Normalize();
	}

	return NormalizedRotation;
}

void UCinematicDirectorSubsystem::DrawDebugAnchorTransform(const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform) const
{
	if (!Request.bDrawDebugAnchor)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Location = AnchorWorldTransform.GetLocation();
	const FRotator Rotation = AnchorWorldTransform.Rotator();
	const float Scale = FMath::Max(1.0f, Request.DebugDrawScale);
	const float Duration = FMath::Max(0.0f, Request.DebugDrawDuration);

	DrawDebugCoordinateSystem(World, Location, Rotation, Scale, false, Duration, 0, 2.0f);
	DrawDebugSphere(World, Location, 16.0f, 16, FColor::Cyan, false, Duration, 0, 1.5f);
}

void UCinematicDirectorSubsystem::CollectParticipants(const FCinematicPlaybackRequest& Request)
{
	ActiveParticipants.Reset();

	AddActorParticipants(Request.InstigatorActor);
	AddActorParticipants(Request.SubjectActor);

	for (AActor* ParticipantActor : Request.AdditionalParticipants)
	{
		AddActorParticipants(ParticipantActor);
	}

	if (!Request.bAffectAllParticipants)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AddActorParticipants(*It);
	}
}

void UCinematicDirectorSubsystem::AddActorParticipants(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	AddParticipantObject(Actor);

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		AddParticipantObject(Component);
	}
}

void UCinematicDirectorSubsystem::AddParticipantObject(UObject* Object)
{
	if (!IsValid(Object) || !Object->GetClass()->ImplementsInterface(UCinematicParticipant::StaticClass()))
	{
		return;
	}

	ActiveParticipants.AddUnique(Object);
}

void UCinematicDirectorSubsystem::NotifyParticipantsStarted()
{
	for (UObject* Participant : ActiveParticipants)
	{
		if (IsValid(Participant))
		{
			ICinematicParticipant::Execute_OnCinematicStarted(Participant, ActiveContext);
		}
	}
}

void UCinematicDirectorSubsystem::NotifyParticipantsEnded()
{
	for (int32 Index = ActiveParticipants.Num() - 1; Index >= 0; --Index)
	{
		UObject* Participant = ActiveParticipants[Index];
		if (IsValid(Participant))
		{
			ICinematicParticipant::Execute_OnCinematicEnded(Participant, ActiveContext);
		}
	}
}
