#include "Character/Enemy/EnemyFSMComponent.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "Character/Enemy/EnemyActionDataAsset.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"

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

void UEnemyFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerCharacter) return;

	switch (CurrentState)
	{
		case EEnemyState::Idle:
			UpdateIdle();
			break;
		case EEnemyState::Approach:
			UpdateApproach();
			break;
		case EEnemyState::Return:
			UpdateReturn();
			break;
		default:
			break;
	}
}

void UEnemyFSMComponent::ChangeState(EEnemyState NewState)
{
	if (CurrentState == NewState) return;
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
		if (Data->DefaultAttackMontage)
		{
			UAnimInstance* AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(Data->DefaultAttackMontage, Data->AttackPlayRate);
			}
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
	
	float AttackRange = OwnerCharacter->GetEnemyData()->AttackRange;
	float DistanceToPlayer = FVector::Dist(OwnerCharacter->GetActorLocation(), TargetPlayer->GetActorLocation());
	
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