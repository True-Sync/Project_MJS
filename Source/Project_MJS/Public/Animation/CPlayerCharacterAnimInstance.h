#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Player/CPlayerCharacter.h"
#include "CPlayerCharacterAnimInstance.generated.h"

/*
- 전투 캐릭터용 AnimInstance 베이스 클래스
- ACombatCharacterActor 전용
- 전투 컴포넌트(Groggy, Motion, HP 등)를 캐싱하여 ABP에 노출
 */
UCLASS()
class PROJECT_MJS_API UCPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Animation|References")
	TObjectPtr<ACPlayerCharacter> OwningPlayerCharacter = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|References")
	TObjectPtr<UCharacterMovementComponent> MovementComp = nullptr;


	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float GroundSpeed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float MovementDirection = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsAccelerating = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsGroggy = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsCombatMotionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsDead = false;
};
