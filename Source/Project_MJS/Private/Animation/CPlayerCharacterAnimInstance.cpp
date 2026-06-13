// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CPlayerCharacterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

void UCPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OwningPlayerCharacter = Cast<ACPlayerCharacter>(TryGetPawnOwner());
	if (!OwningPlayerCharacter)
		return;
	
	MovementComp = OwningPlayerCharacter->GetCharacterMovement();
}

void UCPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwningPlayerCharacter || !MovementComp)
		return;
	
	const FVector Velocity = MovementComp->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	MovementDirection = CalculateDirection(Velocity, OwningPlayerCharacter->GetActorRotation());
	bIsInAir = MovementComp->IsFalling();
	bIsAccelerating = MovementComp->GetCurrentAcceleration().SizeSquared() > 0.f;
}
