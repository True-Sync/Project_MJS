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

	// 언리얼 엔진 표준 데미지 시스템 오버라이드
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
};
