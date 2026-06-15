#include "Character/Enemy/EnemyCharacter.h"

#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AIController.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f) return ActualDamage;
	
	FVector DirectionToAttacker = FVector::ZeroVector;
	if (DamageCauser)
	{
		DirectionToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	// 피격 애니메이션 재생
	UAnimMontage* SelectedMontage = HitMontageFront; // 기본값은 정면 피격

	if (!DirectionToAttacker.IsNearlyZero())
	{
		float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToAttacker);
		float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToAttacker);

		// ForwardDot 값에 따른 판별 (1.0 = 완전 정면, -1.0 = 완전 후면)
		// 0.5 이상이면 대략 전방 120도 안쪽에서 맞은 것으로 간주
		if (ForwardDot >= 0.5f)
		{
			SelectedMontage = HitMontageFront;  // 앞쪽에서 맞음 (뒤로 젖혀지는 모션)
		}
		else if (ForwardDot <= -0.5f)
		{
			SelectedMontage = HitMontageBack;   // 뒤쪽에서 맞음 (앞으로 쏠리는 모션)
		}
		else 
		{
			// 앞/뒤가 아니라면 측면 타격. RightDot이 양수면 오른쪽, 음수면 왼쪽
			if (RightDot > 0.0f)
			{
				SelectedMontage = HitMontageRight; // 오른쪽에서 맞음 (왼쪽으로 기우는 모션)
			}
			else
			{
				SelectedMontage = HitMontageLeft;  // 왼쪽에서 맞음 (오른쪽으로 기우는 모션)
			}
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
	
	// Hit Flash 피격시 반짝임
	if (HitFlashMaterial && GetMesh())
	{
		GetMesh()->SetOverlayMaterial(HitFlashMaterial);
		
		GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AEnemyCharacter::ClearHitFlash, HitFlashDuration, false);
	}
	
	// 피격 넉백 수행
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

		KnockbackDirection.Z += 0.25f;
		KnockbackDirection.Normalize();

		LaunchCharacter(KnockbackDirection * HitBackForce, true, true);
		
		GetWorldTimerManager().SetTimer(HitRecoveryTimerHandle, this, &AEnemyCharacter::ResetHitState, RecoveryTime, false);
	}

	return ActualDamage;
}

void AEnemyCharacter::ResetHitState()
{
	bIsHitBacking = false;
	
	// TODO: 블랙보드 변수(예: bIsStunned)를 제어하거나 AI Behavior Tree를 재개하는 로직
}

void AEnemyCharacter::ClearHitFlash()
{
	if (GetMesh())
	{
		GetMesh()->SetOverlayMaterial(nullptr);
	}
}