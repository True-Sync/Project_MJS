#include "Interaction/InteractionComponent.h"

#include "GameFramework/Pawn.h"
#include "Interaction/Interactable.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetCurrentInteractable(nullptr);
	InteractablesInRange.Reset();
	Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::RegisterInteractable(AActor* InteractableActor)
{
	if (!IsValid(InteractableActor) || !InteractableActor->Implements<UInteractable>())
	{
		return;
	}

	InteractablesInRange.Add(InteractableActor);
	RefreshCurrentInteractable();
}

void UInteractionComponent::UnregisterInteractable(AActor* InteractableActor)
{
	if (!InteractableActor)
	{
		return;
	}

	if (IsValid(InteractableActor) && InteractableActor->Implements<UInteractable>())
	{
		IInteractable::Execute_SetInteractionPromptVisible(InteractableActor, false);
	}
	InteractablesInRange.Remove(InteractableActor);
	RefreshCurrentInteractable();
}

bool UInteractionComponent::TryInteract()
{
	RefreshCurrentInteractable();

	AActor* InteractableActor = CurrentInteractable.Get();
	APawn* OwningPawn = GetOwningPawn();
	if (!IsValid(InteractableActor) || !OwningPawn)
	{
		return false;
	}

	IInteractable::Execute_Interact(InteractableActor, OwningPawn);
	return true;
}

void UInteractionComponent::SetInteractionPromptEnabled(bool bEnabled)
{
	if (bInteractionPromptEnabled == bEnabled)
	{
		return;
	}

	bInteractionPromptEnabled = bEnabled;
	if (AActor* InteractableActor = CurrentInteractable.Get())
	{
		IInteractable::Execute_SetInteractionPromptVisible(InteractableActor, bInteractionPromptEnabled);
	}
}

void UInteractionComponent::RefreshCurrentInteractable()
{
	APawn* OwningPawn = GetOwningPawn();
	AActor* ClosestInteractable = nullptr;
	float ClosestDistanceSquared = TNumericLimits<float>::Max();

	for (auto It = InteractablesInRange.CreateIterator(); It; ++It)
	{
		AActor* Candidate = It->Get();
		if (!IsInteractableCandidate(Candidate))
		{
			It.RemoveCurrent();
			continue;
		}

		if (!OwningPawn || !IInteractable::Execute_CanInteract(Candidate, OwningPawn))
		{
			continue;
		}

		const float DistanceSquared = OwningPawn
			? FVector::DistSquared(OwningPawn->GetActorLocation(), Candidate->GetActorLocation())
			: 0.0f;

		if (!ClosestInteractable || DistanceSquared < ClosestDistanceSquared)
		{
			ClosestInteractable = Candidate;
			ClosestDistanceSquared = DistanceSquared;
		}
	}

	SetCurrentInteractable(ClosestInteractable);
}

void UInteractionComponent::SetCurrentInteractable(AActor* NewInteractable)
{
	AActor* PreviousInteractable = CurrentInteractable.Get();
	if (PreviousInteractable == NewInteractable)
	{
		if (NewInteractable)
		{
			IInteractable::Execute_SetInteractionPromptVisible(NewInteractable, bInteractionPromptEnabled);
		}
		return;
	}

	if (PreviousInteractable)
	{
		IInteractable::Execute_SetInteractionPromptVisible(PreviousInteractable, false);
	}

	CurrentInteractable = NewInteractable;
	if (NewInteractable)
	{
		IInteractable::Execute_SetInteractionPromptVisible(NewInteractable, bInteractionPromptEnabled);
	}
}

bool UInteractionComponent::IsInteractableCandidate(AActor* Candidate) const
{
	return IsValid(Candidate)
		&& Candidate->Implements<UInteractable>();
}

APawn* UInteractionComponent::GetOwningPawn() const
{
	return Cast<APawn>(GetOwner());
}
