#include "Character/Player/CPlayerCharacter.h"

#include "Character/Player/CPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bOrientRotationToMovement = true;
		MovementComponent->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	}
}

void ACPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACPlayerCharacter::Move(const FVector2D& MoveInput)
{
	if (MoveInput.IsNearlyZero())
		return;
	
	const ACPlayerController* PlayerController = Cast<ACPlayerController>(GetController());
	const FRotator YawRot = PlayerController ? PlayerController->GetCameraYawRotation() : FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
	
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	
	AddMovementInput(Forward, MoveInput.Y);
	AddMovementInput(Right, MoveInput.X);
}
