#include "Character/Enemy/EnemyAnimInstance.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "KismetAnimationLibrary.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<AEnemyCharacter>(TryGetPawnOwner());
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter) return;
	
	FVector Velocity = OwnerCharacter->GetVelocity();
	Velocity.Z = 0.0f; // 수직 속도 제외
	Speed = Velocity.Size();
	
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());
	
	bIsDead = false;
}