#include "Character/Enemy/EnemyCharacter.h"

#include "AIController.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/Player/Component/AttackComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	TargetPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TargetPoint"));
	TargetPointComponent->SetupAttachment(RootComponent);
	TargetPointComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TargetPointHeight));
}

void AEnemyCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (TargetPointComponent)
	{
		TargetPointComponent->SetRelativeLocation(FVector(0.0f, 0.0f, TargetPointHeight));
	}
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

FVector AEnemyCharacter::GetTargetPointLocation() const
{
	return TargetPointComponent ? TargetPointComponent->GetComponentLocation() : GetActorLocation();
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f)
	{
		return ActualDamage;
	}

	FVector DirectionToAttacker = FVector::ZeroVector;
	if (DamageCauser)
	{
		DirectionToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	UAnimMontage* SelectedMontage = HitMontageFront;
	if (!DirectionToAttacker.IsNearlyZero())
	{
		const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToAttacker);
		const float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToAttacker);

		if (ForwardDot >= 0.5f)
		{
			SelectedMontage = HitMontageFront;
		}
		else if (ForwardDot <= -0.5f)
		{
			SelectedMontage = HitMontageBack;
		}
		else
		{
			SelectedMontage = RightDot > 0.0f ? HitMontageRight : HitMontageLeft;
		}
	}

	float RecoveryTime = 0.4f;
	if (SelectedMontage)
	{
		UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SelectedMontage);
			RecoveryTime = SelectedMontage->GetPlayLength();
		}
	}

	if (HitFlashMaterial && GetMesh())
	{
		GetMesh()->SetOverlayMaterial(HitFlashMaterial);
		GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AEnemyCharacter::ClearHitFlash, HitFlashDuration, false);
	}

	FVector KnockbackDirection = FVector::ZeroVector;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		KnockbackDirection = PointDamageEvent->ShotDirection;
	}
	else if (DamageCauser)
	{
		KnockbackDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();
	}

	if (!KnockbackDirection.IsNearlyZero())
	{
		bIsHitBacking = true;
		if (AAIController* AIController = Cast<AAIController>(GetController()))
		{
			AIController->StopMovement();
		}

		float AppliedKnockbackForce = HitBackForce;
		if (DamageCauser)
		{
			UAttackComponent* AttackComp = Cast<UAttackComponent>(DamageCauser->GetComponentByClass(UAttackComponent::StaticClass()));
			if (AttackComp)
			{
				AppliedKnockbackForce = AttackComp->GetCurrentKnockbackForce();
			}
		}

		KnockbackDirection.Z += 0.25f;
		KnockbackDirection.Normalize();
		LaunchCharacter(KnockbackDirection * AppliedKnockbackForce, true, true);

		GetWorldTimerManager().SetTimer(HitRecoveryTimerHandle, this, &AEnemyCharacter::ResetHitState, RecoveryTime, false);
	}

	return ActualDamage;
}

void AEnemyCharacter::ResetHitState()
{
	bIsHitBacking = false;
}

void AEnemyCharacter::ClearHitFlash()
{
	if (GetMesh())
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}
