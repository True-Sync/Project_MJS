#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyActionDataAsset.generated.h"

class UAnimMontage;
class UMaterialInterface;

UENUM(BlueprintType)
enum class EEnemyAttackType : uint8
{
	Melee		UMETA(DisplayName = "근접 공격 (Melee)"),
	Dash		UMETA(DisplayName = "돌진 공격 (Dash)"),
	Ranged		UMETA(DisplayName = "원거리 투사체 (Ranged)")
};

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
	
	// ===== 이동 속도 =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Movement")
	float PatrolSpeed = 200.0f; // 대기 및 정찰 시 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Movement")
	float ChaseSpeed = 500.0f; // 추적 시 속도

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Movement")
	float ReturnSpeed = 800.0f; // 어그로 해제 후 스폰 위치 복귀 속도

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
	
	// ===== 전투/공격 방식  =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Attack")
	EEnemyAttackType AttackType = EEnemyAttackType::Melee;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActionData|Attack")
	float BaseDamage = 20.0f;
	
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