#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "System/VFX/VFXTypes.h"
#include "AttackComponent.generated.h"

class UVFXExecutorComponent;
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
	
	UFUNCTION(BlueprintCallable, Category = "Combat|VFX")
	void StartSwordTrail();

	UFUNCTION(BlueprintCallable, Category = "Combat|VFX")
	void EndSwordTrail();
	
protected:
	virtual void BeginPlay() override;

private:
	bool PlayCombo(int32 ComboIndex);
	void PlayQueuedCombo();
	bool HasNextCombo() const;
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	
	
	//VFX 
	UVFXExecutorComponent* ResolveVFXExecutor();
	FVFXExecuteContext MakeAttackVFXContext(const FVector AttackDirection);
	void StartAttackOneShotVFX(const FVector& AttackDirection);
	void StartAttackLoopVFX(const FVector& AttackDirection);
	void StopAttackLoopVFX();
	void FinishAttackVFX();
	

	UPROPERTY(Transient)
	TObjectPtr<UVFXExecutorComponent> CachedVFXExecutor = nullptr;
	
	UPROPERTY(Transient)
	FVFXHandle AttackLoopVFXHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UComboAttackDataAsset> DA_BasicComboAttack;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveMontage;

	bool bIsAttacking = false;
	bool bCanQueueCombo = false;
	bool bComboQueued = false;
	int32 CurrentComboIndex = INDEX_NONE;
	
	//=============== 공격 판정 용 함수 구현부 ==================
private:
	FVector LastWeaponStartPos;
	FVector LastWeaponEndPos;
	FName CurrentKickSocket;
	
	// 현재 타격의 넉백 수치를 기억할 변수
	float CurrentKnockbackForce = 500.0f;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	float GetCurrentKnockbackForce() const { return CurrentKnockbackForce; }

	// ===== Debug / DevConsole용 상태 조회 =====
	bool IsAttacking() const { return bIsAttacking; }
	int32 GetCurrentComboIndex() const { return CurrentComboIndex; }
	bool CanQueueCombo() const { return bCanQueueCombo; }
	bool IsComboQueued() const { return bComboQueued; }

	bool IsWeaponAttacking() const { return bIsWeaponAttacking; }
	bool IsKickAttacking() const { return bIsKickAttacking; }
	
	// 노티파이 상태에서 호출할 공격 판정 시작/종료 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartWeaponAttack(float KnockbackForce);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndWeaponAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartKickAttack(FName SocketName, float KnockbackForce);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndKickAttack();

private:
	
	// 이전에 이미 타격한 대상을 중복 타격하지 않기 위한 리스트
	TSet<TWeakObjectPtr<AActor>> HitActors;

	
protected:
	void CheckWeaponTrace();
	void CheckKickTrace();
	
	bool bIsWeaponAttacking = false;
	bool bIsKickAttacking = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bShowDebugShape = true;
	
	void DrawDebugAttackShape(const FVector& StartPos, const FVector& EndPos, float Radius, bool bHit);
};
