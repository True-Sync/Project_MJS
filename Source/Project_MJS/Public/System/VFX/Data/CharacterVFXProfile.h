#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "System/VFX/VFXTypes.h"
#include "CharacterVFXProfile.generated.h"


UCLASS(BlueprintType)
class PROJECT_MJS_API UCharacterVFXProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	
	// 게임 플레이 태그마다 사용할 VFX 설정 변수.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TMap<FGameplayTag, FVFXDefinition> Definitions;
	
	// 태그와 정확히 맞는 설정 찾기
	const FVFXDefinition* FindDefinition(const FGameplayTag& Tag) const;
};
