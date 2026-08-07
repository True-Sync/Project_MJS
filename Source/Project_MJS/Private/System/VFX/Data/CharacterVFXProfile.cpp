#include "System/VFX/Data/CharacterVFXProfile.h"

const FVFXDefinition* UCharacterVFXProfile::FindDefinition(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
		return nullptr;
	
	return Definitions.Find(Tag);	
}
