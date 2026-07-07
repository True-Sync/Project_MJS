#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// UE Damage 파이프라인(ApplyPointDamage 등)에서 넘어온 실제 데미지 처리
	bool ReceiveDamage(float DamageAmount, AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Combat|Health")
	bool Heal(float HealAmount);

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	bool IsAlive() const { return !bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetHealthPercent() const;

	// Owning actor constructor에서 CDO 기본값을 덮어쓸 때 사용 (플레이어/적 분기)
	void ConfigureDeathBehavior(bool bInDisableInputOnDeath, bool bInDestroyOwnerOnDeath);

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Health")
	FOnDeathSignature OnDeath;

protected:
	virtual void BeginPlay() override;

private:
	void ApplyHealthDelta(float Delta, AController* InstigatedBy, AActor* DamageCauser);
	void HandleDeath(AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	float CurrentHealth = 100.0f;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health", meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	// 플레이어 사망 시 possess PC 입력만 끔 (싱글)
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Health")
	bool bDisableInputOnDeath = true;

	// 적 등: 사망 후 Destroy (플레이어 BP에서는 false 권장)
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Health")
	bool bDestroyOwnerOnDeath = false;
};
