#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class APawn;

UINTERFACE(BlueprintType)
class PROJECT_MJS_API UInteractable : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_MJS_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(APawn* InteractingPawn) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(APawn* InteractingPawn);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void SetInteractionPromptVisible(bool bVisible);
};
