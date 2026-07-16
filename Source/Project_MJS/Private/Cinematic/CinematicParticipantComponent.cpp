#include "Cinematic/CinematicParticipantComponent.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UCinematicParticipantComponent::UCinematicParticipantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCinematicParticipantComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ActiveLockCount > 0)
	{
		ActiveLockCount = 0;
		ReleaseAnimationLocks();
		ReleaseTickLocks();
		ReleaseMovementLock();
		ReleaseAILocks();
		ReleaseInputLocks();
	}

	Super::EndPlay(EndPlayReason);
}

void UCinematicParticipantComponent::OnCinematicStarted_Implementation(const FCinematicPlaybackContext& Context)
{
	++ActiveLockCount;
	if (ActiveLockCount > 1)
	{
		return;
	}

	ApplyInputLocks(Context);
	ApplyAILocks();
	ApplyMovementLock();
	ApplyTickLocks();
	ApplyAnimationLocks();
}

void UCinematicParticipantComponent::OnCinematicEnded_Implementation(const FCinematicPlaybackContext& Context)
{
	if (ActiveLockCount <= 0)
	{
		return;
	}

	--ActiveLockCount;
	if (ActiveLockCount > 0)
	{
		return;
	}

	ReleaseAnimationLocks();
	ReleaseTickLocks();
	ReleaseMovementLock();
	ReleaseAILocks();
	ReleaseInputLocks();
}

void UCinematicParticipantComponent::ApplyInputLocks(const FCinematicPlaybackContext& Context)
{
	LockedPlayerController = ResolvePlayerController(Context);
	if (!LockedPlayerController)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UCinematicInputLockSubsystem* InputLockSubsystem = World->GetSubsystem<UCinematicInputLockSubsystem>())
		{
			InputLockHandle = InputLockSubsystem->AcquireInputLock(LockedPlayerController, bLockPlayerMoveInput, bLockPlayerLookInput, bLockPlayerGameplayInput);
			return;
		}
	}

	if (bLockPlayerMoveInput)
	{
		LockedPlayerController->SetIgnoreMoveInput(true);
		bMoveInputLocked = true;
	}

	if (bLockPlayerLookInput)
	{
		LockedPlayerController->SetIgnoreLookInput(true);
		bLookInputLocked = true;
	}
}

void UCinematicParticipantComponent::ReleaseInputLocks()
{
	if (InputLockHandle != INDEX_NONE)
	{
		if (UWorld* World = GetWorld())
		{
			if (UCinematicInputLockSubsystem* InputLockSubsystem = World->GetSubsystem<UCinematicInputLockSubsystem>())
			{
				InputLockSubsystem->ReleaseInputLock(InputLockHandle);
			}
		}

		InputLockHandle = INDEX_NONE;
		LockedPlayerController = nullptr;
		return;
	}

	if (LockedPlayerController)
	{
		if (bMoveInputLocked)
		{
			LockedPlayerController->SetIgnoreMoveInput(false);
			bMoveInputLocked = false;
		}

		if (bLookInputLocked)
		{
			LockedPlayerController->SetIgnoreLookInput(false);
			bLookInputLocked = false;
		}
	}

	LockedPlayerController = nullptr;
}

void UCinematicParticipantComponent::ApplyMovementLock()
{
	if (!bDisableCharacterMovement)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
	if (!MovementComponent)
	{
		return;
	}

	SavedMovementMode = MovementComponent->MovementMode;
	SavedCustomMovementMode = MovementComponent->CustomMovementMode;
	bMovementModeSaved = true;

	MovementComponent->StopMovementImmediately();
	MovementComponent->DisableMovement();
}

void UCinematicParticipantComponent::ReleaseMovementLock()
{
	if (!bMovementModeSaved)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* MovementComponent = OwnerCharacter ? OwnerCharacter->GetCharacterMovement() : nullptr;
	if (MovementComponent)
	{
		if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(SavedMovementMode, SavedCustomMovementMode);
		}
	}

	bMovementModeSaved = false;
}

void UCinematicParticipantComponent::ApplyTickLocks()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (bDisableOwnerTick)
	{
		bOwnerTickWasEnabled = Owner->IsActorTickEnabled();
		Owner->SetActorTickEnabled(false);
	}

	if (!bDisableComponentTicks)
	{
		return;
	}

	TickLockedComponents.Reset();
	SavedComponentTickStates.Reset();

	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		if (!Component || Component == this)
		{
			continue;
		}

		TickLockedComponents.Add(Component);
		SavedComponentTickStates.Add(Component->IsComponentTickEnabled());
		Component->SetComponentTickEnabled(false);
	}
}

void UCinematicParticipantComponent::ReleaseTickLocks()
{
	AActor* Owner = GetOwner();
	if (Owner && bDisableOwnerTick)
	{
		Owner->SetActorTickEnabled(bOwnerTickWasEnabled);
		bOwnerTickWasEnabled = false;
	}

	for (int32 Index = 0; Index < TickLockedComponents.Num(); ++Index)
	{
		UActorComponent* Component = TickLockedComponents[Index];
		if (Component)
		{
			const bool bWasTickEnabled = SavedComponentTickStates.IsValidIndex(Index) && SavedComponentTickStates[Index];
			Component->SetComponentTickEnabled(bWasTickEnabled);
		}
	}

	TickLockedComponents.Reset();
	SavedComponentTickStates.Reset();
}

void UCinematicParticipantComponent::ApplyAILocks()
{
	if (!bPauseAILogic)
	{
		return;
	}

	PausedBrainComponents.Reset();

	if (AAIController* OwnerAIController = Cast<AAIController>(GetOwner()))
	{
		OwnerAIController->StopMovement();
		TryPauseBrainComponent(OwnerAIController->GetBrainComponent());
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
	if (AIController)
	{
		AIController->StopMovement();
		TryPauseBrainComponent(AIController->GetBrainComponent());
	}
}

void UCinematicParticipantComponent::ReleaseAILocks()
{
	for (UBrainComponent* BrainComponent : PausedBrainComponents)
	{
		if (BrainComponent && BrainComponent->IsPaused())
		{
			BrainComponent->ResumeLogic(TEXT("CinematicEnded"));
		}
	}

	PausedBrainComponents.Reset();
}

void UCinematicParticipantComponent::TryPauseBrainComponent(UBrainComponent* BrainComponent)
{
	if (!BrainComponent || !BrainComponent->IsRunning() || BrainComponent->IsPaused())
	{
		return;
	}

	BrainComponent->PauseLogic(TEXT("CinematicStarted"));
	PausedBrainComponents.AddUnique(BrainComponent);
}

void UCinematicParticipantComponent::ApplyAnimationLocks()
{
	if (!bPauseSkeletalAnimations)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	PausedSkeletalMeshes.Reset();
	SavedMeshPauseStates.Reset();

	TArray<USkeletalMeshComponent*> MeshComponents;
	Owner->GetComponents(MeshComponents);
	for (USkeletalMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		PausedSkeletalMeshes.Add(MeshComponent);
		SavedMeshPauseStates.Add(MeshComponent->bPauseAnims);
		MeshComponent->bPauseAnims = true;
	}
}

void UCinematicParticipantComponent::ReleaseAnimationLocks()
{
	for (int32 Index = 0; Index < PausedSkeletalMeshes.Num(); ++Index)
	{
		USkeletalMeshComponent* MeshComponent = PausedSkeletalMeshes[Index];
		if (MeshComponent)
		{
			const bool bWasPaused = SavedMeshPauseStates.IsValidIndex(Index) && SavedMeshPauseStates[Index];
			MeshComponent->bPauseAnims = bWasPaused;
		}
	}

	PausedSkeletalMeshes.Reset();
	SavedMeshPauseStates.Reset();
}

APlayerController* UCinematicParticipantComponent::ResolvePlayerController(const FCinematicPlaybackContext& Context) const
{
	if (Context.PlayerController)
	{
		return Context.PlayerController;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
}
