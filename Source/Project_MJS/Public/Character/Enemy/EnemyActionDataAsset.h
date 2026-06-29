#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyActionDataAsset.generated.h"

class UAnimMontage;
class UMaterialInterface;

UCLASS(BlueprintType)
class PROJECT_MJS_API UEnemyActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// ===== FSM & AI =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float AttackRange = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float LeashDistance = 3000.0f; // 이 거리를 벗어나면 어그로 해제 후 복귀

	// ===== 강인도 (슈퍼아머) =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Poise")
	bool bHasSuperArmor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Poise", meta = (EditCondition = "bHasSuperArmor"))
	float MaxPoise = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Poise", meta = (EditCondition = "bHasSuperArmor"))
	float GroggyDuration = 5.0f;

	// ===== 피격 및 넉백 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Hit")
	float HitBackForce = 1300.0f;

	// ===== 애니메이션 몽타주 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageFront;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageBack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageRight;
	
	// ===== 공격 애니메이션 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Animation")
	TObjectPtr<class UAnimMontage> DefaultAttackMontage;

	// 공격 애니메이션의 재생 속도 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Animation")
	float AttackPlayRate = 1.0f;

	// ===== 피격 피드백 =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	TObjectPtr<UMaterialInterface> HitFlashMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	float HitFlashDuration = 0.1f;
};