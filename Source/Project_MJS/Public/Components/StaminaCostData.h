#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StaminaCostData.generated.h"

// 스태미나 소모량 데이터 구조체 (스킬 연동용)
USTRUCT(BlueprintType)
struct FStaminaCostData
{
	GENERATED_BODY()

	// 기본 소모량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	float StaminaCost = 10.0f;

	// 소모 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	bool bCanConsume = true;

	// 소모 시 필요한 태그 (예: 특정 상태 효과) - 비어있으면 무조건 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	FGameplayTagContainer RequiredTags;

	// 소모 시 제외되는 태그 (예: 이미 스태미나 고갈 상태) - 하나라도 있으면 불가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	FGameplayTagContainer ExcludedTags;

	// [NEW] 소모 중 회복량 (점프, 달리기 등 지속적 소모 시)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	float StaminaRegenRate = 0.0f;

	// [NEW] 스킬 사용 시 최대 스태미나 증가량 (일시적 버프)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	float StaminaMaxIncrease = 0.0f;

	// [NEW] 스킬 지속 시간 (일시적 스태미나 증가 효과)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	float StaminaDuration = 0.0f;

	// [NEW] 소모 시 적용되는 상태 효과 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	FGameplayTagContainer EffectTags;

	// [NEW] 소모 시 제거되는 상태 효과 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Cost")
	FGameplayTagContainer RemovedEffectTags;
};