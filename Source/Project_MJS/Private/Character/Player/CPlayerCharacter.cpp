#include "Character/Player/CPlayerCharacter.h"

#include "Character/Player/CPlayerController.h"
#include "Character/Player/Component/AttackComponent.h"
#include "Character/Player/Component/SkillComponent.h"
#include "Character/Player/Component/DodgeComponent.h"
#include "Character/Player/Component/PlayerMovementComponent.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Cinematic/CinematicActionComponent.h"
#include "Cinematic/CinematicParticipantComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"

ACPlayerCharacter::ACPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;
	
	PlayerMovementComponent = Cast<UPlayerMovementComponent>(GetCharacterMovement());
	DodgeComponent = CreateDefaultSubobject<UDodgeComponent>(TEXT("DodgeComponent"));
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("SkillComponent"));
	TargetingComponent = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));
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

float ACPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DodgeComponent)
	{
		if (DodgeComponent->TryConsumeJustDodge(DamageCauser))
		{
			return 0.0f;
		}

		if (DodgeComponent->IsDodgeInvincible())
		{
			return 0.0f;
		}
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
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

bool ACPlayerCharacter::RequestDodge()
{
	if (!DodgeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: DodgeComponent is missing."));
		return false;
	}

	return DodgeComponent->RequestDodge();
}


bool ACPlayerCharacter::RequestSkill(USkillDataAsset* SkillData)
{
	if (!SkillComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestSkill failed: SkillComponent is missing."));
		return false;
	}

	return SkillComponent->ActivateSkill(SkillData);
}

bool ACPlayerCharacter::GetLastMoveWorldDirection(FVector& OutDirection) const
{
	OutDirection = LastMoveWorldDirection;
	return bHasMoveInput;
}
