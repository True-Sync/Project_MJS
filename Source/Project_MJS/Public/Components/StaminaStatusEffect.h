#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StaminaStatusEffect.generated.h"

// 스태미나 상태 효과 데이터 구조체
USTRUCT(BlueprintType)
struct FStaminaStatusEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Status")
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Status")
	float Duration = 5.0f; // 초

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Status")
	float StaminaModifier = 0.0f; // 양수: 증가, 음수: 감소

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Status")
	bool bApplyOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Status")
	bool bRemoveOnEnd = true;
};

// 스태미나 상태 효과 컴포넌트
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UStaminaStatusEffect : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaStatusEffect();

	// 상태 효과 적용
	void ApplyStatusEffect(const FStaminaStatusEffectData& Data);

	// 상태 효과 제거
	void RemoveStatusEffect(const FGameplayTagContainer& Tags);

	// 현재 적용된 상태 효과 목록 조회
	TArray<FStaminaStatusEffectData> GetActiveStatusEffects() const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateStatusEffects(float DeltaTime);

	// 현재 적용된 상태 효과 목록
	UPROPERTY(Transient)
	TArray<FStaminaStatusEffectData> ActiveStatusEffects;
};