#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"

class UAnimMontage;
class UComboAttackDataAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_MJS_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttackComponent();

	void RequestAttack();
	void SetComboWindowOpen(bool bOpen);

protected:
	virtual void BeginPlay() override;

private:
	bool PlayCombo(int32 ComboIndex);
	void PlayQueuedCombo();
	bool HasNextCombo() const;
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UComboAttackDataAsset> DA_BasicComboAttack;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage;

	bool bIsAttacking = false;
	bool bCanQueueCombo = false;
	bool bComboQueued = false;
	int32 CurrentComboIndex = INDEX_NONE;
};
