#include "Cinematic/CinematicActionComponent.h"

#include "Cinematic/CinematicDirectorSubsystem.h"
#include "Cinematic/CinematicTypes.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "LevelSequence.h"

UCinematicActionComponent::UCinematicActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UCinematicActionComponent::PlayCinematic(ULevelSequence* Sequence, bool bAffectAllParticipants, bool bRestoreViewTarget, float BlendOutTime)
{
	FCinematicPlaybackRequest Request;
	Request.Sequence = Sequence;
	Request.InstigatorActor = GetOwner();
	Request.SubjectActor = GetOwner();
	Request.ParticipantScope = bAffectAllParticipants ? ECinematicParticipantScope::AllInWorld : ECinematicParticipantScope::ExplicitOnly;
	Request.bRestoreViewTarget = bRestoreViewTarget;
	Request.BlendOutTime = FMath::Max(0.0f, BlendOutTime);

	return SubmitCinematicRequest(Request);
}

bool UCinematicActionComponent::PlayCinematicRequest(FCinematicPlaybackRequest Request)
{
	return SubmitCinematicRequest(Request);
}

bool UCinematicActionComponent::PlayAnchoredCinematic(
	ULevelSequence* Sequence,
	ECinematicAnchorMode AnchorMode,
	ECinematicRotationSource RotationSource,
	FTransform RelativeTransform,
	bool bUseYawOnly,
	bool bAffectAllParticipants,
	bool bRestoreViewTarget,
	float BlendOutTime)
{
	FCinematicPlaybackRequest Request;
	Request.Sequence = Sequence;
	Request.InstigatorActor = GetOwner();
	Request.SubjectActor = GetOwner();
	Request.ParticipantScope = bAffectAllParticipants ? ECinematicParticipantScope::AllInWorld : ECinematicParticipantScope::ExplicitOnly;
	Request.bRestoreViewTarget = bRestoreViewTarget;
	Request.BlendOutTime = FMath::Max(0.0f, BlendOutTime);
	Request.AnchorMode = AnchorMode;
	Request.RotationSource = RotationSource;
	Request.RelativeTransform = RelativeTransform;
	Request.bUseYawOnly = bUseYawOnly;

	return SubmitCinematicRequest(Request);
}

void UCinematicActionComponent::StopCinematic()
{
	UWorld* World = GetWorld();
	if (UCinematicDirectorSubsystem* DirectorSubsystem = World ? World->GetSubsystem<UCinematicDirectorSubsystem>() : nullptr)
	{
		DirectorSubsystem->StopCinematic();
	}
}

bool UCinematicActionComponent::IsCinematicPlaying() const
{
	UWorld* World = GetWorld();
	const UCinematicDirectorSubsystem* DirectorSubsystem = World ? World->GetSubsystem<UCinematicDirectorSubsystem>() : nullptr;
	return DirectorSubsystem && DirectorSubsystem->IsCinematicPlaying();
}

bool UCinematicActionComponent::SubmitCinematicRequest(FCinematicPlaybackRequest& Request) const
{
	UWorld* World = GetWorld();
	UCinematicDirectorSubsystem* DirectorSubsystem = World ? World->GetSubsystem<UCinematicDirectorSubsystem>() : nullptr;
	if (!DirectorSubsystem)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("PlayCinematic failed: CinematicDirectorSubsystem is missing."));
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Request.InstigatorActor)
	{
		Request.InstigatorActor = Owner;
	}

	if (!Request.SubjectActor)
	{
		Request.SubjectActor = Request.InstigatorActor ? Request.InstigatorActor.Get() : Owner;
	}

	if (!Request.PlayerController)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			Request.PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
		}
	}

	static const FName PlayerBindingTag(TEXT("Player"));

	AActor* PlayerBindingActor = Request.InstigatorActor ? Request.InstigatorActor.Get() : Owner;

	if (IsValid(PlayerBindingActor) && !Request.BindingOverrides.ContainsByPredicate(
		[](const FCinematicBindingOverride& Override)
		{
			static const FName LocalPlayerBindingTag(TEXT("Player"));
			return Override.BindingTag == LocalPlayerBindingTag;
		}))
	{
		FCinematicBindingOverride PlayerBinding;
		PlayerBinding.BindingTag = PlayerBindingTag;
		PlayerBinding.Actors.Add(PlayerBindingActor);
		PlayerBinding.bAllowBindingsFromAsset = false;
		Request.BindingOverrides.Add(PlayerBinding);
	}

	Request.BlendOutTime = FMath::Max(0.0f, Request.BlendOutTime);
	return DirectorSubsystem->PlayCinematic(Request);
}
