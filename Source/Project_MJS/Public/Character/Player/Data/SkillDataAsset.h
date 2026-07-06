#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SkillDataAsset.generated.h"

class UAnimMontage;
class ULevelSequence;

// 스킬 연출 타입 구분
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	//일반 스킬
	Normal UMETA(DisplayName = "Normal"),
	//궁극기
	Ultimate UMETA(DisplayName = "Ultimate"),
};

UCLASS(BlueprintType, Blueprintable)
class PROJECT_MJS_API USkillDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	FText SkillName;

	// 연출 타입 (Normal / Ultimate)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	ESkillType Type = ESkillType::Normal;

	// 시네마틱(Level Sequence) 사용 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic")
	bool bUseCinematic = true;

	// 스킬 발동 시 재생할 애니메이션 몽타주 (선택)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	// 연결된 Level Sequence 에셋 (시네마틱 연출용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	TObjectPtr<ULevelSequence> CinematicSequence = nullptr;
};
