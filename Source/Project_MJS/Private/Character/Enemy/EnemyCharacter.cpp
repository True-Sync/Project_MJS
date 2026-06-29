#include "Character/Enemy/EnemyCharacter.h"
#include "Character/Enemy/EnemyFSMComponent.h"
#include "Character/Enemy/EnemyActionDataAsset.h"
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

	FSMComponent = CreateDefaultSubobject<UEnemyFSMComponent>(TEXT("FSMComponent"));
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

	// 데이터 에셋이 연결되어 있다면 강인도를 초기화
	if (EnemyDataAsset)
	{
		CurrentPoise = EnemyDataAsset->MaxPoise;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyDataAsset is missing on %s!"), *GetName());
	}
}

FVector AEnemyCharacter::GetTargetPointLocation() const
{
	return TargetPointComponent ? TargetPointComponent->GetComponentLocation() : GetActorLocation();
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.0f || !EnemyDataAsset)
	{
		return ActualDamage;
	}

	// 1. 강인도(슈퍼아머) 및 보스 판정 로직
	if (EnemyDataAsset->bHasSuperArmor && !bIsGroggy)
	{
		CurrentPoise -= ActualDamage;

		if (EnemyDataAsset->HitFlashMaterial && GetMesh())
		{
			GetMesh()->SetOverlayMaterial(EnemyDataAsset->HitFlashMaterial);
			GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AEnemyCharacter::ClearHitFlash, EnemyDataAsset->HitFlashDuration, false);
		}

		if (CurrentPoise <= 0.0f)
		{
			bIsGroggy = true;
			if (FSMComponent) FSMComponent->ChangeState(EEnemyState::Stagger);
			GetWorldTimerManager().SetTimer(PoiseRecoveryTimerHandle, this, &AEnemyCharacter::RecoverPoise, EnemyDataAsset->GroggyDuration, false);
		}
		else
		{
			return ActualDamage; // 강인도가 남았다면 데미지만 적용 (넉백 무시)
		}
	}
	else if (FSMComponent && !EnemyDataAsset->bHasSuperArmor)
	{
		FSMComponent->ChangeState(EEnemyState::Stagger);
	}

	// 2. 피격 몽타주 재생 및 넉백 로직
	FVector DirectionToAttacker = FVector::ZeroVector;
	if (DamageCauser)
	{
		DirectionToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	}

	UAnimMontage* SelectedMontage = EnemyDataAsset->HitMontageFront;
	if (!DirectionToAttacker.IsNearlyZero())
	{
		const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToAttacker);
		const float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToAttacker);

		if (ForwardDot >= 0.5f) SelectedMontage = EnemyDataAsset->HitMontageFront;
		else if (ForwardDot <= -0.5f) SelectedMontage = EnemyDataAsset->HitMontageBack;
		else SelectedMontage = RightDot > 0.0f ? EnemyDataAsset->HitMontageRight : EnemyDataAsset->HitMontageLeft;
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

	if (EnemyDataAsset->HitFlashMaterial && GetMesh() && !EnemyDataAsset->bHasSuperArmor) 
	{
		GetMesh()->SetOverlayMaterial(EnemyDataAsset->HitFlashMaterial);
		GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &AEnemyCharacter::ClearHitFlash, EnemyDataAsset->HitFlashDuration, false);
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

		float AppliedKnockbackForce = EnemyDataAsset->HitBackForce;
		if (DamageCauser)
		{
			UAttackComponent* AttackComp = Cast<UAttackComponent>(DamageCauser->GetComponentByClass(UAttackComponent::StaticClass()));
			if (AttackComp) AppliedKnockbackForce = AttackComp->GetCurrentKnockbackForce();
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
	if (FSMComponent && !bIsGroggy)
	{
		FSMComponent->ChangeState(EEnemyState::Idle);
	}
}

void AEnemyCharacter::RecoverPoise()
{
	if (EnemyDataAsset)
	{
		CurrentPoise = EnemyDataAsset->MaxPoise;
	}
	bIsGroggy = false;
	
	if (FSMComponent)
	{
		FSMComponent->ChangeState(EEnemyState::Idle);
	}
}

void AEnemyCharacter::ClearHitFlash()
{
	if (GetMesh()) GetMesh()->SetOverlayMaterial(nullptr);
}