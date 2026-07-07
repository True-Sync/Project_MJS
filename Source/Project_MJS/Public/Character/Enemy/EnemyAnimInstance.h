#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

UCLASS()
class PROJECT_MJS_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsDead;

private:
	UPROPERTY()
	class AEnemyCharacter* OwnerCharacter;
};