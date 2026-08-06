#include "Character/Enemy/EnemyFSMComponent.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "Character/Enemy/EnemyActionDataAsset.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Enemy/GroupCombatSubsystem.h"

UEnemyFSMComponent::UEnemyFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnemyFSMComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AEnemyCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerAIController = Cast<AAIController>(OwnerCharacter->GetController());
		SpawnLocation = OwnerCharacter->GetActorLocation(); // 최초 스폰 위치 저장
	}

	TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void UEnemyFSMComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseTokenIfHasOne();
	
	Super::EndPlay(EndPlayReason);
}

void UEnemyFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter) return;

	if (TokenCooldownTime > 0.0f)
	{
		TokenCooldownTime -= DeltaTime;
	}
	
	if (CurrentState == EEnemyState::Approach)
	{
		if (OwnerCharacter->GetVelocity().Size() < 50.0f)
		{
			ApproachTime += DeltaTime;
		}
		else
		{
			ApproachTime = 0.0f; 
		}
	}

	switch (CurrentState)
	{
	case EEnemyState::Idle: UpdateIdle(); break;
	case EEnemyState::Approach: UpdateApproach(); break;
	case EEnemyState::Return: UpdateReturn(); break;
	case EEnemyState::Circling: UpdateCircling(); break; 
	default: break;
	}
}

void UEnemyFSMComponent::ChangeState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;

	if (!OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;
	UEnemyActionDataAsset* Data = OwnerCharacter->GetEnemyData();
	
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	
	if (CurrentState == EEnemyState::Attack_Telegraph || 
		CurrentState == EEnemyState::Attack_Execution || 
		CurrentState == EEnemyState::Attack_Recovery)
	{
		TokenCooldownTime = Data->AttackTokenCooldown; 
	}

	if (NewState == EEnemyState::Attack_Recovery || 
		NewState == EEnemyState::Idle || 
		NewState == EEnemyState::Circling || 
		NewState == EEnemyState::Return || 
		NewState == EEnemyState::Stagger) 
	{
		ReleaseTokenIfHasOne();
	}

	if (NewState == EEnemyState::Approach) 
	{
		ApproachTime = 0.0f;
		float DistToPlayer = TargetPlayer ? FVector::Dist(OwnerCharacter->GetActorLocation(), TargetPlayer->GetActorLocation()) : 0.0f;
		
		// 거리가 Threshold 이상이면 돌진 모드 ON!
		if (DistToPlayer >= Data->DashAttackThreshold)
		{
			bIsPreparingDashAttack = true;
			if (MovementComp) MovementComp->MaxWalkSpeed = Data->DashSpeed;
		}
		// 일반 접근
		else
		{
			bIsPreparingDashAttack = false;
			if (MovementComp) MovementComp->MaxWalkSpeed = Data->ChaseSpeed;
		}
	}
	else if (NewState == EEnemyState::Circling || NewState == EEnemyState::Idle)
	{
		bIsPreparingDashAttack = false;
		if (MovementComp) MovementComp->MaxWalkSpeed = Data->PatrolSpeed;
	}
	else if (NewState == EEnemyState::Return)
	{
		bIsPreparingDashAttack = false;
		if (MovementComp) MovementComp->MaxWalkSpeed = Data->ReturnSpeed;
	}

	switch (CurrentState)
	{
	case EEnemyState::Attack_Telegraph:
		if (OwnerAIController) OwnerAIController->StopMovement();
			
		if (Data->AttackPatterns.Num() > 0)
		{
			FEnemyAttackPattern SelectedPattern = Data->AttackPatterns[0]; 
			bool bPatternSelected = false;
			
			if (bIsPreparingDashAttack)
			{
				for (const FEnemyAttackPattern& Pattern : Data->AttackPatterns)
				{
					if (Pattern.AttackType == EEnemyAttackType::Dash)
					{
						SelectedPattern = Pattern;
						bPatternSelected = true;
						break;
					}
				}
			}
			
			if (!bPatternSelected)
			{
				float TotalWeight = 0.0f;
				for (const FEnemyAttackPattern& Pattern : Data->AttackPatterns) TotalWeight += Pattern.ProbabilityWeight;

				float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
				float AccumulatedWeight = 0.0f;

				for (const FEnemyAttackPattern& Pattern : Data->AttackPatterns)
				{
					AccumulatedWeight += Pattern.ProbabilityWeight;
					if (RandomValue <= AccumulatedWeight)
					{
						SelectedPattern = Pattern; 
						break;
					}
				}
			}
			
			bIsPreparingDashAttack = false;

			if (SelectedPattern.AttackMontage)
			{
				UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					float PlayDuration = AnimInstance->Montage_Play(SelectedPattern.AttackMontage, SelectedPattern.AttackPlayRate);
		
					if (PlayDuration <= 0.0f)
					{
						ChangeState(EEnemyState::Idle);
					}
				}
			}
			else
			{
				ChangeState(EEnemyState::Idle);
			}
		}
		else
		{
			ChangeState(EEnemyState::Idle); // 패턴 데이터가 없을 때의 방어
		}
		break;

	case EEnemyState::Stagger:
		if (OwnerAIController) OwnerAIController->StopMovement();
		// 피격 몽타주는 EnemyCharacter의 TakeDamage에서 처리함
		break;

	default:
		break;
	}
}

bool UEnemyFSMComponent::CheckLeashDistance()
{
	if (!OwnerCharacter || !OwnerCharacter->GetEnemyData()) return false;
	
	float LeashDistance = OwnerCharacter->GetEnemyData()->LeashDistance;
	float DistanceFromSpawn = FVector::Dist(SpawnLocation, OwnerCharacter->GetActorLocation());
	
	if (DistanceFromSpawn > LeashDistance)
	{
		ChangeState(EEnemyState::Return);
		return true;
	}
	return false;
}

void UEnemyFSMComponent::UpdateCircling()
{
	if (!TargetPlayer || !OwnerAIController || !OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;
	
	UEnemyActionDataAsset* Data = OwnerCharacter->GetEnemyData();
	OwnerAIController->SetFocus(TargetPlayer);

	float DesiredDistance = Data->AttackRange * Data->CirclingDistanceMultiplier; 
	float CurrentDistance = FVector::Dist(OwnerCharacter->GetActorLocation(), TargetPlayer->GetActorLocation());

	if (TokenCooldownTime <= 0.0f && CurrentDistance <= DesiredDistance + 100.0f)
	{
		UGroupCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UGroupCombatSubsystem>();
		if (CombatSubsystem && CombatSubsystem->RequestAttackToken(OwnerCharacter, TargetPlayer))
		{
			OwnerAIController->ClearFocus(EAIFocusPriority::Gameplay);
			ChangeState(EEnemyState::Approach);
			return;
		}
	}

	// 단순 대기 및 거리 유지
	if (CurrentDistance > DesiredDistance)
	{
		OwnerAIController->MoveToActor(TargetPlayer, DesiredDistance);
	}
	else
	{
		OwnerAIController->StopMovement();
	}
}

void UEnemyFSMComponent::ReleaseTokenIfHasOne()
{
	UWorld* World = GetWorld();
	if (!World || !OwnerCharacter) 
	{
		return;
	}
	
	if (UGroupCombatSubsystem* CombatSubsystem = World->GetSubsystem<UGroupCombatSubsystem>())
	{
		CombatSubsystem->ReleaseAttackToken(OwnerCharacter);
	}
}

void UEnemyFSMComponent::UpdateIdle()
{
	if (!TargetPlayer || !OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;
	
	float DistanceToPlayer = FVector::Dist(OwnerCharacter->GetActorLocation(), TargetPlayer->GetActorLocation());
	
	if (DistanceToPlayer <= OwnerCharacter->GetEnemyData()->LeashDistance)
	{
		ChangeState(EEnemyState::Approach);
	}
}

void UEnemyFSMComponent::UpdateApproach()
{
	if (!TargetPlayer || !OwnerAIController || !OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;

	UEnemyActionDataAsset* Data = OwnerCharacter->GetEnemyData();
	
	if (CheckLeashDistance()) return;

	UGroupCombatSubsystem* CombatSubsystem = GetWorld()->GetSubsystem<UGroupCombatSubsystem>();
	if (!CombatSubsystem) return;

	if (TokenCooldownTime > 0.0f || !CombatSubsystem->RequestAttackToken(OwnerCharacter, TargetPlayer))
	{
		ChangeState(EEnemyState::Circling);
		return;
	}

	float AttackRange = OwnerCharacter->GetEnemyData()->AttackRange;
	float DistanceToPlayer = FVector::Dist(OwnerCharacter->GetActorLocation(), TargetPlayer->GetActorLocation());
	
	if (DistanceToPlayer > Data->AttackRange && ApproachTime > Data->ApproachTimeout)
	{
		TokenCooldownTime = Data->YieldTokenCooldown; 
		ChangeState(EEnemyState::Circling);
		return;
	}

	if (DistanceToPlayer <= Data->AttackRange)
	{
		OwnerAIController->StopMovement();
		ChangeState(EEnemyState::Attack_Telegraph);
	}
	else
	{
		OwnerAIController->MoveToActor(TargetPlayer, 50.0f);
	}
}

void UEnemyFSMComponent::UpdateReturn()
{
	if (!OwnerAIController) return;

	float DistanceToSpawn = FVector::Dist(OwnerCharacter->GetActorLocation(), SpawnLocation);
	if (DistanceToSpawn <= 100.0f)
	{
		ChangeState(EEnemyState::Idle);
	}
	else
	{
		OwnerAIController->MoveToLocation(SpawnLocation, 50.0f);
	}
}