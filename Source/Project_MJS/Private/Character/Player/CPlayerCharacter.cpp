#include "Character/Player/CPlayerCharacter.h"

#include "Character/Player/CPlayerController.h"
#include "Character/Player/Component/AttackComponent.h"
#include "Character/Player/Component/SkillComponent.h"
#include "Character/Player/Component/DodgeComponent.h"
#include "Character/Player/Component/PlayerMovementComponent.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Cinematic/CinematicActionComponent.h"
#include "Cinematic/CinematicParticipantComponent.h"
#include "Character/SharedComponent/HealthComponent.h"
#include "Character/SharedComponent/StaminaComponent.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "System/VFX/VFXExecutorComponent.h"
#include "System/VFX/VFXGameplayTags.h"
#include "GameFramework/PlayerController.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"

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
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	VFXExcutorComponent = CreateDefaultSubobject<UVFXExecutorComponent>(TEXT("VFXExecutorComponent"));
	
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

	UE_LOG(LogTemp, Log, TEXT("[VFX Test] Player BeginPlay"));
}

float ACPlayerCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	
	if (DamageAmount <= 0.0f)
		return 0.0f;
	
	if (!IsValid(HealthComponent) || !HealthComponent->IsAlive())
		return 0.0f;

	//회피 계산 이후 피해 판정.
	if (IsValid(DodgeComponent))
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
	
	// 실제로 적중한 공격만 표준 데미지 이벤트를 발생
	const float ActualDamage =
		Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.0f)
		return 0.0f;

	if (!HealthComponent->ReceiveDamage(ActualDamage,EventInstigator,DamageCauser))
		return 0.0f;

	
	return ActualDamage;
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

void ACPlayerCharacter::ResetState()
{
	if (!GetWorld()) return;

	// HP 최대화
	HealFull();

	// 이동 상태 초기화
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = 600.0f;
		Movement->Velocity = FVector::ZeroVector;
	}

	// 공격/회피 상태 초기화 (공격 중/콤보 큐 등)
	if (UAttackComponent* AttackComp = GetAttackComponent())
	{
		AttackComp->SetComboWindowOpen(false);
	}

	// 타겟팅 초기화
	if (UTargetingComponent* TargetComp = GetTargetingComponent())
	{
		TargetComp->ClearHardTarget();
	}

	// 입력 잠금 해제 (사망 DisableInput + 시네마틱 lock)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		EnableInput(PC);

		if (UCinematicInputLockSubsystem* InputLockSub = GetWorld()->GetSubsystem<UCinematicInputLockSubsystem>())
		{
			InputLockSub->ReleaseAllInputLocksForPlayer(PC);
		}
		else
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[DevConsole] player.reset_state: Player state reset."));
}

void ACPlayerCharacter::HealFull()
{
	if (UHealthComponent* HealthComp = GetHealthComponent())
	{
		const float MaxHP = HealthComp->GetMaxHealth();
		const float CurrentHP = HealthComp->GetCurrentHealth();
		if (CurrentHP < MaxHP)
		{
			HealthComp->Heal(MaxHP - CurrentHP);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[DevConsole] player.heal_full: HP set to max."));
}

void ACPlayerCharacter::Revive()
{
	if (!GetWorld()) return;

	UHealthComponent* HealthComp = GetHealthComponent();
	if (!HealthComp) return;

	// 사망 상태라면 HP 최대화 + 상태 초기화
	if (!HealthComp->IsAlive())
	{
		HealthComp->Heal(HealthComp->GetMaxHealth());
	}

	ResetState();

	UE_LOG(LogTemp, Log, TEXT("[DevConsole] player.revive: Player revived."));
}
