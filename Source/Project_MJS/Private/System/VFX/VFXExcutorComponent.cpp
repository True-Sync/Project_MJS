#include "System/VFX/VFXExcutorComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "System/VFX/Data/CharacterVFXProfile.h"

UVFXExcutorComponent::UVFXExcutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVFXExcutorComponent::ExecuteVFX(const FGameplayTag& Tag, const FVFXExecuteContext& Context)
{
	if (!Tag.IsValid() && !CharacterVfxProfile)
		return;
	
	const FVFXDefinition* Definition = CharacterVfxProfile->FindDefinition(Tag);
	
	if (!Definition)
		return;
}



