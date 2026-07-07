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
		ApproachTime += DeltaTime;
	}

	switch (CurrentState)
	{
	case EEnemyState::Idle: UpdateIdle(); break;
	case EEnemyState::Approach: UpdateApproach(); break;
	case EEnemyState::Return: UpdateReturn(); break;
	case EEnemyState::Circling: UpdateCircling(); break; // 대기 틱 실행
	default: break;
	}
}

void UEnemyFSMComponent::ChangeState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
	
	if (NewState == EEnemyState::Approach)
	{
		ApproachTime = 0.0f;
	}
	
	if (CurrentState == EEnemyState::Attack_Telegraph || 
			CurrentState == EEnemyState::Attack_Execution || 
			CurrentState == EEnemyState::Attack_Recovery)
	{
		TokenCooldownTime = 3.0f;
	}
	
	if (NewState == EEnemyState::Idle || 
			NewState == EEnemyState::Circling || 
			NewState == EEnemyState::Return || 
			NewState == EEnemyState::Stagger ||
			NewState == EEnemyState::Attack_Recovery) 
	{
		ReleaseTokenIfHasOne();
	}
	
	CurrentState = NewState;

	if (!OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;

	UEnemyActionDataAsset* Data = OwnerCharacter->GetEnemyData();

	switch (CurrentState)
	{
	case EEnemyState::Idle:
		if (OwnerAIController) OwnerAIController->StopMovement();
		break;

	case EEnemyState::Attack_Telegraph:
		if (OwnerAIController) OwnerAIController->StopMovement();
			
		// === 공격 애니메이션(몽타주) 실행 ===
		switch (Data->AttackType)
		{
		case EEnemyAttackType::Melee:
			if (Data->DefaultAttackMontage)
			{
				UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
				if (AnimInstance)
				{
					AnimInstance->Montage_Play(Data->DefaultAttackMontage, Data->AttackPlayRate);
				}
			}
			break;

		case EEnemyAttackType::Dash:
			// TODO: 돌진 공격 연출
			break;

		case EEnemyAttackType::Ranged:
			// TODO: 원거리 공격
			break;
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

	OwnerAIController->SetFocus(TargetPlayer);

	float DesiredDistance = OwnerCharacter->GetEnemyData()->AttackRange * 1.5f; 
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

	// ===== 거리 유지 (Spacing) 로직 =====
	//if (CurrentDistance < DesiredDistance - 50.0f)
	//{
	//	FVector BackwardsDir = (OwnerCharacter->GetActorLocation() - TargetPlayer->GetActorLocation()).GetSafeNormal();
	//	FVector TargetLoc = TargetPlayer->GetActorLocation() + (BackwardsDir * DesiredDistance);
	//	OwnerAIController->MoveToLocation(TargetLoc);
	//}
	//else if (CurrentDistance > DesiredDistance + 50.0f)
	//{
	//	OwnerAIController->MoveToActor(TargetPlayer, DesiredDistance);
	//}
	//else
	//{
	//	OwnerAIController->StopMovement();
	//}
	
	if (CurrentDistance > DesiredDistance)
	{
		OwnerAIController->MoveToActor(TargetPlayer, DesiredDistance);
	}
	else
	{
		// 적정 거리에 들어오면 그 자리에 멈춰서 쿨타임이 돌 때까지 노려봄
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
	if (TargetPlayer)
	{
		ChangeState(EEnemyState::Approach);
	}
}

void UEnemyFSMComponent::UpdateApproach()
{
	if (!TargetPlayer || !OwnerAIController || !OwnerCharacter || !OwnerCharacter->GetEnemyData()) return;

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
	
	if (DistanceToPlayer > AttackRange && ApproachTime > 3.0f)
	{
		TokenCooldownTime = 2.0f;
		ChangeState(EEnemyState::Circling);
		return;
	}
	
	if (DistanceToPlayer <= AttackRange)
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