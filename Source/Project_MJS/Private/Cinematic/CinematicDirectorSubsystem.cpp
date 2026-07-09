#include "Cinematic/CinematicDirectorSubsystem.h"
#include "MovieSceneObjectBindingID.h"
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
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogCinematicSystem);

bool UCinematicDirectorSubsystem::PlayCinematic(const FCinematicPlaybackRequest& Request)
{
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
		// 1. LocalOnly, AuthorityOnly, AnyNetMode 같은 값으로 서버에서 재생할지, 클라에서만 재생할지 따지는데...어짜피 로컬임.
		UE_LOG(LogCinematicSystem, Verbose, TEXT("PlayCinematic skipped by network policy. Sequence=%s Policy=%d"), *GetNameSafe(Request.Sequence), static_cast<int32>(Request.NetworkPolicy));
		return false;
	}

	if (ActiveSequencePlayer)
	{
		// 2. bStopPreviousCinematic == false면 새 컷신 요청을 거절.
		// bStopPreviousCinematic == true면 기존 컷신을 강제로 종료하고 새 컷신 시작.
		if (!Request.bStopPreviousCinematic)
		{
			UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic rejected: another cinematic is already active."));
			return false;
		}

		FinishCinematic(true);
	}

	// 3. CreateLevelSequencePlayer는 내부적으로 ALevelSequenceActor를 월드에 만들고, 그 시퀀스를 재생할 ULevelSequencePlayer를 반환.
	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	
	ALevelSequenceActor* CreatedSequenceActor = nullptr;
	ActiveSequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		World,
		Request.Sequence,
		PlaybackSettings,
		CreatedSequenceActor
	);

	ActiveSequenceActor = CreatedSequenceActor; 
	// 4. 여기서 생성되는 놈 두가지 저장 : ULevelSequencePlayer -> 실제 재생기 /ALevelSequenceActor -> 월드에 배치된 시퀀스 액터

	if (!ActiveSequencePlayer)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic failed: could not create LevelSequencePlayer. Sequence=%s"), *GetNameSafe(Request.Sequence));
		ActiveSequenceActor = nullptr;
		return false;
	}
	
	// 5. 여기까지 왔으면 시퀀서 재생기 생성자체는 성공

	ActiveSequencePlayer->SetCompletionModeOverride(
		EMovieSceneCompletionModeOverride::ForceKeepState 
		/* 
		 * 6.시퀀스가 끝났을 때 트랙이 적용한 상태를 유지하겠다는 뜻
		 * 시퀀스에서 액터 위치, 카메라, 가시성, 머티리얼 값 등을 바꿨다면 기본적으로 끝나고 원래대로 돌아갈 수도 있는데, 
		 * ForceKeepState는 “끝난 상태를 유지” 쪽으로 강제
		 */
	); 

	// 7. 컷신 시작 전에 현재 카메라 ViewTarget을 저장
	ActivePlayerController = ResolvePlayerController(Request);
	PreviousViewTarget = ActivePlayerController ? ActivePlayerController->GetViewTarget() : nullptr;
	ActiveBlendOutTime = FMath::Max(0.0f, Request.BlendOutTime);
	bShouldRestoreViewTarget = Request.bRestoreViewTarget;

	// 8. 동적 앵커 위치를 계산하고 적용. 시퀀스를 그냥 에셋에 저장된 위치에서 재생할 수도 있고, 플레이어 위치나 타겟 방향 기준으로 재생할 수도 있음.
	FTransform AnchorWorldTransform = FTransform::Identity;
	bool bAppliedDynamicTransform = false;
	ApplyDynamicTransform(Request, AnchorWorldTransform, bAppliedDynamicTransform);
	ApplyBindingOverrides(Request); // 그 다음 바인딩 오버라이드를 적용하고 아래 디버그 앵커 그림
	DrawDebugAnchorTransform(Request, bAppliedDynamicTransform ? AnchorWorldTransform : (ActiveSequenceActor ? ActiveSequenceActor->GetActorTransform() : FTransform::Identity));

	// 9. 현재 재생 컨텍스트를 만들고서 참가자 수집, 참가자들 컷신 시작대기
	BuildActiveContext(Request, AnchorWorldTransform, bAppliedDynamicTransform);
	CollectParticipants(Request);
	NotifyParticipantsStarted();

	// 10. 그 다음 끝났을 때 호출될 콜백을 등록해서 끝나면 HandleSequenceFinished 자동 호출, 실제 컷신 시작. 
	ActiveSequencePlayer->OnFinished.AddDynamic(this, &UCinematicDirectorSubsystem::HandleSequenceFinished);
	ActiveSequencePlayer->Play();

	UE_LOG(LogCinematicSystem, Log, TEXT("PlayCinematic succeeded: Sequence=%s Participants=%d"), *GetNameSafe(Request.Sequence), ActiveParticipants.Num());
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

	const bool bNaturalFinish = !bStopPlayback;

	TWeakObjectPtr<APawn> PawnToPreserve;
	FTransform CinematicEndTransform = FTransform::Identity;
	bool bShouldPreservePawnTransform = false;

	if (bNaturalFinish && ActivePlayerController)
	{
		APawn* Pawn = ActivePlayerController->GetPawn();
		if (IsValid(Pawn))
		{
			PawnToPreserve = Pawn;
			CinematicEndTransform = Pawn->GetActorTransform();
			bShouldPreservePawnTransform = true;

			UE_LOG(LogCinematicSystem, Log,
				TEXT("Cached real player pawn cinematic end transform. Pawn=%s Location=%s Rotation=%s"),
				*GetNameSafe(Pawn),
				*CinematicEndTransform.GetLocation().ToCompactString(),
				*CinematicEndTransform.Rotator().ToCompactString());
		}
	}

	NotifyParticipantsEnded();

	if (bShouldPreservePawnTransform)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimerForNextTick([PawnToPreserve, CinematicEndTransform]()
			{
				APawn* Pawn = PawnToPreserve.Get();
				if (!IsValid(Pawn))
				{
					return;
				}

				const bool bTeleported = Pawn->TeleportTo(
					CinematicEndTransform.GetLocation(),
					CinematicEndTransform.Rotator(),
					false,
					true
				);

				UE_LOG(LogCinematicSystem, Log,
					TEXT("Preserved player cinematic end transform %s. Pawn=%s Location=%s Rotation=%s"),
					bTeleported ? TEXT("succeeded") : TEXT("failed"),
					*GetNameSafe(Pawn),
					*CinematicEndTransform.GetLocation().ToCompactString(),
					*CinematicEndTransform.Rotator().ToCompactString());
			});
		}
	}

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
	if (!bShouldRestoreViewTarget || !ActivePlayerController)
	{
		return;
	}

	AActor* TargetToRestore = nullptr;

	// 1. 저장된 PreviousViewTarget이 유효하면 사용 (IsValid는 PendingKill까지 체크)
	if (AActor* PrevActor = Cast<AActor>(PreviousViewTarget))
	{
		if (IsValid(PrevActor))
		{
			TargetToRestore = PrevActor;
		}
	}

	// 2. 유효하지 않다면 현재 플레이어 컨트롤러의 Pawn을 폴백으로 사용
	if (!TargetToRestore)
	{
		if (APawn* CurrentPawn = ActivePlayerController->GetPawn())
		{
			if (IsValid(CurrentPawn))
			{
				TargetToRestore = CurrentPawn;
			}
		}
	}

	// 3. 최종적으로 타겟이 있으면 부드럽게 복구
	if (TargetToRestore)
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
		// 시네마틱 요청자가 넘긴 컨트롤러가 있으면 그대로 그 컨트롤러를 넘김(리턴)
		return Request.PlayerController;
	}

	if (const APawn* InstigatorPawn = Cast<APawn>(Request.InstigatorActor))
	{
		// 직접 PlayerController가 없으면, InstigatorActor가 Pawn인지 확인함.
		// 예를 들어 스킬 컷신이면 InstigatorActor는 보통 플레이어 캐릭터일 가능성이 높기 때문.
		if (APlayerController* PlayerController = Cast<APlayerController>(InstigatorPawn->GetController()))
		{
			return PlayerController;
		}
	}

	if (const APawn* SubjectPawn = Cast<APawn>(Request.SubjectActor))
	{
		// 위에 Instigator에서도 못 찾으면 SubjectActor에서도 똑같이 찾아봄.
		if (APlayerController* PlayerController = Cast<APlayerController>(SubjectPawn->GetController()))
		{
			return PlayerController;
		}
	}

	// fallback : 위에서 다 못 찾으면 월드의 첫 번째 PlayerController를 가져옴
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
			UE_LOG(LogCinematicSystem, Warning, TEXT("Cinematic binding override skipped: Tag=%s has no valid actors."), *BindingOverride.BindingTag.ToString());
			continue;
		}

		const FMovieSceneObjectBindingID ExistingBinding = ActiveSequenceActor->FindNamedBinding(BindingOverride.BindingTag);
		if (!ExistingBinding.IsValid())
		{
			UE_LOG(LogCinematicSystem, Warning,
				TEXT("Cinematic binding override failed: Binding Tag '%s' was not found in the active sequence. If this tag is inside a Subsequence, move the binding/tag to the Master Sequence."),
				*BindingOverride.BindingTag.ToString());
			continue;
		}

		UE_LOG(LogCinematicSystem, Log,
			TEXT("Applying cinematic binding override. Tag=%s ActorCount=%d AllowAssetBinding=%s"),
			*BindingOverride.BindingTag.ToString(),
			BoundActors.Num(),
			BindingOverride.bAllowBindingsFromAsset ? TEXT("true") : TEXT("false"));

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

	switch (Request.ParticipantScope)
	{
	case ECinematicParticipantScope::ExplicitOnly:
		// 이미 Instigator/Subject/AdditionalParticipants만 수집했으므로 종료.
		return;
	case ECinematicParticipantScope::AllInWorld:
	default:
		break;
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

FString UCinematicDirectorSubsystem::GetCinematicStatusSummary() const
{
	TArray<FString> Lines;

	if (!IsCinematicPlaying())
	{
		Lines.Add(TEXT("Cinematic: Not Playing"));
		return FString::Join(Lines, TEXT("\n"));
	}

	Lines.Add(TEXT("Cinematic: Playing"));
	Lines.Add(FString::Printf(TEXT("  Sequence: %s"), *GetNameSafe(ActiveContext.Sequence)));
	Lines.Add(FString::Printf(TEXT("  AnchorMode: %d"), static_cast<int32>(ActiveContext.AnchorMode)));
	Lines.Add(FString::Printf(TEXT("  RotationSource: %d"), static_cast<int32>(ActiveContext.RotationSource)));
	Lines.Add(FString::Printf(TEXT("  Participants: %d"), ActiveParticipants.Num()));

	if (IsValid(ActivePlayerController))
	{
		Lines.Add(TEXT("  InputLocked: Yes"));
	}
	else
	{
		Lines.Add(TEXT("  InputLocked: No"));
	}

	return FString::Join(Lines, TEXT("\n"));
}
