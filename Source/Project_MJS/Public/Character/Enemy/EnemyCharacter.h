#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class USceneComponent;
class UEnemyFSMComponent;
class UEnemyActionDataAsset;

UCLASS()
class PROJECT_MJS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	FVector GetTargetPointLocation() const;

	// ===== FSM (AI 상태 제어기) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Component")
	TObjectPtr<UEnemyFSMComponent> FSMComponent;

	// ===== 데이터 에셋 반환 (FSM 등 외부에서 접근 용이하도록) =====
	UFUNCTION(BlueprintCallable, Category = "AI|Data")
	UEnemyActionDataAsset* GetEnemyData() const { return EnemyDataAsset; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// ===== 데이터 주도 설계 (Data-Driven) =====
	// 하드코딩된 수치들을 제거하고, 데이터 에셋 하나로 바리에이션을 관리합니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI|Data")
	TObjectPtr<UEnemyActionDataAsset> EnemyDataAsset;

	// ===== 강인도 시스템 내부 상태 =====
	float CurrentPoise = 100.0f;
	bool bIsGroggy = false;

private:
	void ResetHitState();
	void ClearHitFlash();
	void RecoverPoise();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TargetPointComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetPointHeight = 55.0f;

	FTimerHandle HitRecoveryTimerHandle;
	FTimerHandle HitFlashTimerHandle;
	FTimerHandle PoiseRecoveryTimerHandle;

	bool bIsHitBacking = false;
};