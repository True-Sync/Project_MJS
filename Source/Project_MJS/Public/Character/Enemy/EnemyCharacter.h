#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAnimMontage;
class UMaterialInterface;
class USceneComponent;
class UHealthComponent;

UCLASS()
class PROJECT_MJS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	FVector GetTargetPointLocation() const;
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// 피격 시 뒤로 밀리는 기본 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HitBackForce = 1300.0f;

	// ===== 방향별 피격 애니메이션 몽타주 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageRight;

	// ===== 피격 피드백 =====
	// 피격 순간 잠깐 적용할 오버레이 머티리얼
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	TObjectPtr<UMaterialInterface> HitFlashMaterial;

	// 피격 오버레이 머티리얼이 유지되는 시간 (보통 0.05초 ~ 0.1초가 가장 타격감이 좋습니다)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	float HitFlashDuration = 0.1f;

private:
	void ResetHitState();
	void ClearHitFlash();

	// 타겟팅 UI와 카메라 포커스가 바라볼 몸통 기준 위치
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TargetPointComponent;

	// TargetPointComponent가 루트 기준으로 떠 있는 높이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting", meta = (AllowPrivateAccess = "true"))
	float TargetPointHeight = 55.0f;

	FTimerHandle HitRecoveryTimerHandle;
	FTimerHandle HitFlashTimerHandle;

	bool bIsHitBacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;
};
