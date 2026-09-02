#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

UCLASS(ClassGroup = (Interaction), meta = (BlueprintSpawnableComponent))
class PROJECT_MJS_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void RegisterInteractable(AActor* InteractableActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UnregisterInteractable(AActor* InteractableActor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	void SetInteractionPromptEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable.Get(); }

private:
	void RefreshCurrentInteractable();
	void SetCurrentInteractable(AActor* NewInteractable);
	bool IsInteractableCandidate(AActor* Candidate) const;
	APawn* GetOwningPawn() const;

	TSet<TWeakObjectPtr<AActor>> InteractablesInRange;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> CurrentInteractable;

	bool bInteractionPromptEnabled = true;
};
