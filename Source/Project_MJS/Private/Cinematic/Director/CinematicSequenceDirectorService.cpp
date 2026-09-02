#include "Cinematic/Director/CinematicSequenceDirectorService.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "DefaultLevelSequenceInstanceData.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneObjectBindingID.h"
#include "MovieSceneSequencePlaybackSettings.h"

static FTransform ResolveActorOrSocketTransform(const AActor* Actor, FName SocketName)
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

static AActor* ResolveAnchorActor(const FCinematicPlaybackRequest& Request)
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

static FRotator NormalizeCinematicRotation(const FRotator& Rotation, bool bUseYawOnly)
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

static FRotator ResolveRotation(
	const FCinematicPlaybackRequest& Request,
	APlayerController* PlayerController,
	const FTransform& AnchorTransform)
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
		if (PlayerController)
		{
			Rotation = PlayerController->GetControlRotation();
		}
		break;
	case ECinematicRotationSource::PlayerCameraRotation:
		if (PlayerController && PlayerController->PlayerCameraManager)
		{
			Rotation = PlayerController->PlayerCameraManager->GetCameraRotation();
		}
		else if (PlayerController)
		{
			Rotation = PlayerController->GetControlRotation();
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

static FTransform ResolveDynamicAnchorTransform(const FCinematicPlaybackRequest& Request, APlayerController* PlayerController)
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

	AnchorTransform.SetRotation(ResolveRotation(Request, PlayerController, AnchorTransform).Quaternion());
	AnchorTransform.NormalizeRotation();

	FTransform FinalTransform = Request.RelativeTransform * AnchorTransform;
	FinalTransform.NormalizeRotation();
	return FinalTransform;
}

static void ApplyBindingOverrides(ALevelSequenceActor* SequenceActor, const FCinematicPlaybackRequest& Request)
{
	if (!SequenceActor)
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

		const FMovieSceneObjectBindingID ExistingBinding = SequenceActor->FindNamedBinding(BindingOverride.BindingTag);
		if (!ExistingBinding.IsValid())
		{
			UE_LOG(LogCinematicSystem, Warning,
				TEXT("Cinematic binding override failed: Binding Tag '%s' was not found in the active sequence. If this tag is inside a Subsequence, move the binding/tag to the Master Sequence."),
				*BindingOverride.BindingTag.ToString());
			continue;
		}

		SequenceActor->SetBindingByTag(BindingOverride.BindingTag, BoundActors, BindingOverride.bAllowBindingsFromAsset);
	}
}

static void DrawDebugAnchorTransform(UWorld* World, const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform)
{
	if (!Request.bDrawDebugAnchor || !World)
	{
		return;
	}

	const float Scale = FMath::Max(1.0f, Request.DebugDrawScale);
	const float Duration = FMath::Max(0.0f, Request.DebugDrawDuration);
	DrawDebugCoordinateSystem(World, AnchorWorldTransform.GetLocation(), AnchorWorldTransform.Rotator(), Scale, false, Duration, 0, 2.0f);
	DrawDebugSphere(World, AnchorWorldTransform.GetLocation(), 16.0f, 16, FColor::Cyan, false, Duration, 0, 1.5f);
}

ULevelSequencePlayer* FCinematicSequenceDirectorService::CreateSequencePlayer(UWorld* World, ULevelSequence* Sequence, ALevelSequenceActor*& OutSequenceActor)
{
	OutSequenceActor = nullptr;
	if (!World || !Sequence)
	{
		return nullptr;
	}

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ULevelSequencePlayer* SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(World, Sequence, PlaybackSettings, OutSequenceActor);
	if (!SequencePlayer)
	{
		DestroySequenceActor(OutSequenceActor);
		OutSequenceActor = nullptr;
		return nullptr;
	}

	SequencePlayer->SetCompletionModeOverride(EMovieSceneCompletionModeOverride::ForceKeepState);
	return SequencePlayer;
}

void FCinematicSequenceDirectorService::ConfigureSequenceActor(
	ALevelSequenceActor* SequenceActor,
	APlayerController* PlayerController,
	const FCinematicPlaybackRequest& Request,
	FTransform& OutAnchorWorldTransform,
	bool& bOutAppliedDynamicTransform)
{
	OutAnchorWorldTransform = FTransform::Identity;
	bOutAppliedDynamicTransform = false;

	if (!SequenceActor)
	{
		return;
	}

	if (Request.AnchorMode != ECinematicAnchorMode::AuthoredWorld)
	{
		OutAnchorWorldTransform = ResolveDynamicAnchorTransform(Request, PlayerController);
		SequenceActor->SetActorTransform(OutAnchorWorldTransform);

		UDefaultLevelSequenceInstanceData* InstanceData = NewObject<UDefaultLevelSequenceInstanceData>(SequenceActor, NAME_None, RF_Transient);
		if (InstanceData)
		{
			InstanceData->TransformOrigin = OutAnchorWorldTransform;
			SequenceActor->DefaultInstanceData = InstanceData;
			SequenceActor->bOverrideInstanceData = true;
		}

		bOutAppliedDynamicTransform = true;
	}

	ApplyBindingOverrides(SequenceActor, Request);
	const FTransform DebugTransform = bOutAppliedDynamicTransform ? OutAnchorWorldTransform : SequenceActor->GetActorTransform();
	DrawDebugAnchorTransform(SequenceActor->GetWorld(), Request, DebugTransform);
}

void FCinematicSequenceDirectorService::DestroySequenceActor(ALevelSequenceActor* SequenceActor)
{
	if (IsValid(SequenceActor))
	{
		SequenceActor->Destroy();
	}
}
