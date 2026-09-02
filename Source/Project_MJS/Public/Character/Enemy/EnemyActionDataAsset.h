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

USTRUCT(BlueprintType)
struct FEnemyAttackPattern
{
	GENERATED_BODY()

	// 이 공격이 발생할 확률 가중치입니다. (예: 10과 5가 있다면 10쪽이 2배 더 자주 발생)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	float ProbabilityWeight = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	EEnemyAttackType AttackType = EEnemyAttackType::Melee;

	// 이 공격이 적중했을 때 들어가는 데미지입니다. (패턴마다 데미지를 다르게 설정 가능!)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	float BaseDamage = 20.0f;

	// 이 패턴에서 재생할 애니메이션 몽타주입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Data")
	float AttackPlayRate = 1.0f;
};

UCLASS(BlueprintType)
class PROJECT_MJS_API UEnemyActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// =========================================================
	// [01] AI 전술 및 행동 패턴 (AI Tactics & Behavior)
	// 몬스터의 지능, 적극성, 포위망 형태를 결정하는 핵심 수치들입니다.
	// =========================================================
	
	// 플레이어를 쫓아가다가 어그로가 풀려 스폰 위치로 돌아가는 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "01. AI Tactics")
	float LeashDistance = 3000.0f;

	// 공격(타격)을 마친 몬스터가 다음 공격 기회(토큰)를 얻기까지 대기하는 시간입니다.
	// ex) 0.5초로 줄이면 쉴 새 없이 몰아치는 광전사가 되고, 5초로 늘리면 신중하게 간을 보는 적이 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "01. AI Tactics")
	float AttackTokenCooldown = 3.0f; 

	// 플레이어에게 다가가는 도중 동료에게 길막을 당했을 때, 접근을 포기하고 대기조로 빠지는 제한 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "01. AI Tactics")
	float ApproachTimeout = 3.0f; 

	// 길막힘으로 공격을 포기한 몬스터가 다시 공격을 시도하기까지의 페널티 대기 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "01. AI Tactics")
	float YieldTokenCooldown = 2.0f; 

	// 토큰이 없는 대기조 몬스터들이 플레이어를 중심으로 포위망을 형성할 때의 거리입니다. (공격 사거리 * 배율)
	// ex) 1.2로 설정하면 촘촘하게 압박하고, 2.5로 설정하면 멀찍이서 빙빙 도는 전술적인 형태가 됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "01. AI Tactics")
	float CirclingDistanceMultiplier = 1.5f;


	// =========================================================
	// [02] 전투 스펙 (Combat Specs)
	// 몬스터의 기본적인 공격 능력치를 결정합니다.
	// =========================================================

	// 공격 방식 설정 (근접, 돌진, 원거리)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "02. Combat Specs")
	EEnemyAttackType AttackType = EEnemyAttackType::Melee;

	// 적이 공격을 시작하기 위해 플레이어에게 접근해야 하는 타겟 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "02. Combat Specs")
	float AttackRange = 250.0f;

	// 플레이어에게 입히는 기본 데미지 수치입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "02. Combat Specs")
	float BaseDamage = 20.0f;


	// =========================================================
	// [03] 이동 속도 (Movement Speed)
	// 상황에 따른 몬스터의 이동 속도를 설정합니다.
	// =========================================================

	// 평상시 순찰하거나 대기조(Circling) 상태에서 걷는 속도입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "03. Movement")
	float PatrolSpeed = 200.0f; 

	// 플레이어를 발견하고 공격을 위해 달려갈 때의 뛰는 속도입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "03. Movement")
	float ChaseSpeed = 500.0f; 

	// 어그로가 풀려 스폰 위치로 복귀할 때의 뛰어가는 속도입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "03. Movement")
	float ReturnSpeed = 800.0f; 


	// =========================================================
	// [04] 슈퍼아머 및 강인도 (Poise & Groggy)
	// 보스급 몬스터나 묵직한 적들의 피격 내성을 설정합니다.
	// =========================================================

	// 체크 시 몬스터가 일반 공격을 맞아도 움찔(피격 애니메이션)하지 않고 버팁니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "04. Super Armor")
	bool bHasSuperArmor = false;

	// 슈퍼아머가 켜져 있을 때 버틸 수 있는 강인도의 총량입니다. (이 수치가 0이 되면 그로기 상태가 됩니다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "04. Super Armor", meta = (EditCondition = "bHasSuperArmor"))
	float MaxPoise = 100.0f;

	// 강인도가 깨져 그로기(Stagger) 상태에 빠졌을 때, 무방비 상태로 멈춰있는 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "04. Super Armor", meta = (EditCondition = "bHasSuperArmor"))
	float GroggyDuration = 5.0f;


	// =========================================================
	// [05] 공격 애니메이션 (Attack Animations)
	// 공격 시 사용할 몽타주와 속도 배율을 관리합니다.
	// =========================================================
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "05. Attack Patterns")
	TArray<FEnemyAttackPattern> AttackPatterns;

	// =========================================================
	// [06] 피격 애니메이션 (Hit Animations)
	// 방향별 피격 리액션 몽타주를 설정합니다.
	// =========================================================

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "06. Hit Animations")
	TObjectPtr<UAnimMontage> HitMontageFront;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "06. Hit Animations")
	TObjectPtr<UAnimMontage> HitMontageBack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "06. Hit Animations")
	TObjectPtr<UAnimMontage> HitMontageLeft;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "06. Hit Animations")
	TObjectPtr<UAnimMontage> HitMontageRight;


	// =========================================================
	// [07] 피격 피드백 (Hit Feedback & Knockback)
	// 타격감과 시각적 피드백 효과를 조율합니다.
	// =========================================================

	// 공격을 받았을 때 몬스터가 뒤로 밀려나는 힘(넉백)의 세기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "07. Hit Feedback")
	float HitBackForce = 1300.0f;

	// 맞았을 때 몬스터가 번쩍이는 효과(깜빡임)에 사용할 머티리얼입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "07. Hit Feedback")
	TObjectPtr<UMaterialInterface> HitFlashMaterial;

	// 머티리얼이 번쩍이고 사라지는 데 걸리는 시간입니다. (보통 0.1 ~ 0.2초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "07. Hit Feedback")
	float HitFlashDuration = 0.1f;
};