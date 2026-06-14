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
	
	//=============== 공격 판정 용 함수 구현부 ==================
private:
	FVector LastWeaponStartPos;
	FVector LastWeaponEndPos;
	FName CurrentKickSocket;
	
public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 노티파이 상태에서 호출할 공격 판정 시작/종료 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartWeaponAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndWeaponAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartKickAttack(FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndKickAttack();
	
	// 이전에 이미 타격한 대상을 중복 타격하지 않기 위한 리스트
	TArray<AActor*> HitActors;

	
protected:
	void CheckWeaponTrace();
	void CheckKickTrace();
	
	bool bIsWeaponAttacking = false;
	bool bIsKickAttacking = false;
	
	// 디버그 표시 여부를 에디터에서 켜고 끌 수 있는 스위치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Debug")
	bool bShowDebugShape = true;

	// 시각적 디버그를 그려주는 전용 함수
	void DrawDebugAttackShape(const FVector& StartPos, const FVector& EndPos, float Radius, bool bHit);
};
