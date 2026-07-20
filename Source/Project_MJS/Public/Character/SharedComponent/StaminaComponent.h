#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Character/SharedData/StaminaCostData.h"
#include "StaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, OldStamina, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepletedSignature);

// 상태 효과 적용, 제거, 복원용 델리게이트들
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectAppliedSignature, FGameplayTagContainer, EffectTags, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectRemovedSignature, FGameplayTagContainer, RemovedTags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxStaminaRestoredSignature, float, OldMaxStamina, float, NewMaxStamina);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStaminaComponent();
	bool CanConsumeStamina(const FStaminaCostData& CostData) const;

	// 스태미나 소모 (공격, 점프 등) - 소모량 데이터 사용
	bool ConsumeStamina(const FStaminaCostData& CostData);

	// 레거시 호환성 - Amount 만 받음 (기본값 사용)
	bool ConsumeStamina(float Amount);

	// 스태미나 회복 (시간 기반 자동 회복)
	void RegenerateStamina(float DeltaTime);
	
	
	
	// ============= Util =============
	
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool IsStaminaFull() const { return CurrentStamina >= MaxStamina; }

	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetStaminaPercent() const;

	// Owning actor constructor 에서 CDO 기본값을 덮어쓸 때 사용 (플레이어/적 분기)
	UFUNCTION(BlueprintCallable, Category = "Combat|Stamina")
	void ConfigureStaminaBehavior(float InRegenRate, bool bInDepleteOnDeath);

	// 스테미나 고갈인지 확인
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool IsStaminaDepleted() const { return bIsDepleted; }

	
	
	// ============= Delegate =============
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnStaminaChangedSignature OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnStaminaDepletedSignature OnStaminaDepleted;

	// 상태 효과 적용 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnEffectAppliedSignature OnEffectApplied;

	// 상태 효과 제거 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnEffectRemovedSignature OnEffectRemoved;

	// 최대 스태미나 복원 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Combat|Stamina")
	FOnMaxStaminaRestoredSignature OnMaxStaminaRestored;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ApplyStaminaDelta(float Delta);
	void HandleStaminaDepletion();

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Stamina", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	float StaminaRegenRate = 20.0f; // 초당 회복량

	// 스태미나 고갈 시 Owning Actor를 Destroy할지 여부 (적 등)
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Stamina")
	bool bDepleteOnDeath = false;
	

	// OnStaminaDepleted 중복 호출 방지를 위한 플래그
	UPROPERTY(Transient)
	bool bIsDepleted = false;
};
