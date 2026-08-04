#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "System/VFX/VFXTypes.h"
#include "VFXExcutorComponent.generated.h"

class UCharacterVFXProfile;

UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class PROJECT_MJS_API UVFXExcutorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFXExcutorComponent();
	
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void ExecuteVFX(const FGameplayTag& Tag, const FVFXExecuteContext& Context);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UCharacterVFXProfile> CharacterVfxProfile;
};