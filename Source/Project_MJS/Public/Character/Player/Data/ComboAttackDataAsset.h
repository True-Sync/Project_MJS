#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ComboAttackDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FComboAttackEntry
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FName SectionName = NAME_None;
};

UCLASS(BlueprintType)
class PROJECT_MJS_API UComboAttackDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FComboAttackEntry> ComboEntries;
};
