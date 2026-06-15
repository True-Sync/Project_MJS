#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

class UAnimMontage;

UCLASS()
class PROJECT_MJS_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

	// 넉백으로 밀려나는 힘의 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HitBackForce = 1300.0f;

	// === 방향별 피격 애니메이션 몽타주 ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Animation")
	TObjectPtr<UAnimMontage> HitMontageRight;

private:
	// 넉백이 끝난 후 상태를 복구하는 함수
	void ResetHitState();

	FTimerHandle HitRecoveryTimerHandle;
	bool bIsHitBacking = false;
	
protected:
	// === 피격 피드백 (머티리얼 오버레이) ===
	
	// 에디터에서 할당할 하얗게 번쩍이는 머티리얼
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	UMaterialInterface* HitFlashMaterial;

	// 번쩍임이 유지되는 시간 (보통 0.05초 ~ 0.1초가 가장 타격감이 좋습니다)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Feedback")
	float HitFlashDuration = 0.1f;

private:
	// 머티리얼 복구를 위한 타이머 핸들
	FTimerHandle HitFlashTimerHandle;

	// 번쩍임 효과를 지우는 함수
	void ClearHitFlash();
};
