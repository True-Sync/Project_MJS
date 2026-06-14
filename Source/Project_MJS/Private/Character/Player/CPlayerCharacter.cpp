#include "Character/Player/CPlayerCharacter.h"

#include "Character/Player/CPlayerController.h"
#include "Character/Player/Component/AttackComponent.h"
#include "Character/Player/Component/DodgeComponent.h"
#include "Cinematic/CinematicActionComponent.h"
#include "Cinematic/CinematicParticipantComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPlayerCharacter::ACPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	DodgeComponent = CreateDefaultSubobject<UDodgeComponent>(TEXT("DodgeComponent"));
	CinematicActionComponent = CreateDefaultSubobject<UCinematicActionComponent>(TEXT("CinematicActionComponent"));
	CinematicParticipantComponent = CreateDefaultSubobject<UCinematicParticipantComponent>(TEXT("CinematicParticipantComponent"));

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
	{
		LastMoveWorldDirection = FVector::ZeroVector;
		bHasMoveInput = false;
		return;
	}
	
	const ACPlayerController* PlayerController = Cast<ACPlayerController>(GetController());
	const FRotator YawRot = PlayerController ? PlayerController->GetCameraYawRotation() : FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
	
	const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
	LastMoveWorldDirection = (Forward * MoveInput.Y + Right * MoveInput.X).GetSafeNormal2D();
	bHasMoveInput = !LastMoveWorldDirection.IsNearlyZero();
	
	AddMovementInput(Forward, MoveInput.Y);
	AddMovementInput(Right, MoveInput.X);
}

void ACPlayerCharacter::RequestAttack()
{
	if (!AttackComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestAttack failed: AttackComponent is missing."));
		return;
	}

	AttackComponent->RequestAttack();
}

void ACPlayerCharacter::RequestDodge()
{
	if (!DodgeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: DodgeComponent is missing."));
		return;
	}

	DodgeComponent->RequestDodge();
}

bool ACPlayerCharacter::GetLastMoveWorldDirection(FVector& OutDirection) const
{
	OutDirection = LastMoveWorldDirection;
	return bHasMoveInput;
}
