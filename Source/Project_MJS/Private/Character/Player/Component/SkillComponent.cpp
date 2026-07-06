#include "Character/Player/Component/SkillComponent.h"

#include "ChaosBreakingEventFilter.h"
#include "Cinematic/CinematicActionComponent.h"
#include "Cinematic/CinematicTypes.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"


USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* Owner = GetOwner())
		CinematicActionComp = Owner->FindComponentByClass<UCinematicActionComponent>();
	
}

bool USkillComponent::ActivateSkill(USkillDataAsset* SkillData)
{
	if (!SkillData || !GetWorld() || !CinematicActionComp)
		return false;
	
	if (IsSkillCinematicPlaying())
		return false;
	
	ActiveSkillData = SkillData;
	StopActiveMontages();
	
	if (SkillData->MontageToPlay)
	{
		AActor* Owner = GetOwner();
		if(ACharacter* Char = Cast<ACharacter>(Owner))
		{
			UAnimInstance* AnimInstance = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr;
			if (IsValid(AnimInstance))
			{
				AnimInstance->Montage_Play(SkillData->MontageToPlay,1.0f);
			}
		}
	}
	
	if (SkillData ->bUseCinematic && SkillData->CinematicSequence)
	{
		PlaySkillCinematic(SkillData);
	}
	
	return true;
}

bool USkillComponent::IsSkillCinematicPlaying() const
{
	// 스킬 데이터가 할당되어 있고, 시네마틱 시스템이 활성 상태면 진행 중으로 간주
	return ActiveSkillData != nullptr && CinematicActionComp->IsCinematicPlaying();
}

void USkillComponent::PlaySkillCinematic(USkillDataAsset* SkillData)
{
	if (!SkillData || !SkillData->CinematicSequence || !CinematicActionComp)
		return;
	
	AActor* Owner = GetOwner();
	FCinematicPlaybackRequest Request;
	
	//시네마틱 플레이 기본 설정
	Request.Sequence = SkillData->CinematicSequence;
	Request.InstigatorActor = Owner;
	Request.SubjectActor = Owner;
	Request.bRestoreViewTarget = true;
	Request.BlendOutTime = 0.15f;
	
	if (SkillData->Type == ESkillType::Ultimate)
	{
		// 궁극기: 전역적 연출 + 기존 컷신 중단 허용
		Request.ParticipantScope = ECinematicParticipantScope::AllInWorld;
		Request.bStopPreviousCinematic = true;
		Request.AnchorMode = ECinematicAnchorMode::SubjectActor;
	}
	
	else
	{
		// 일반 스킬: 명시적 참가자만 사용, 충돌 시 우선순위 낮게 유지
		Request.ParticipantScope = ECinematicParticipantScope::ExplicitOnly;
		Request.bStopPreviousCinematic = false;
		Request.AnchorMode = ECinematicAnchorMode::InstigatorToSubject;
	}

	// CinematicActionComponent를 통해 실제 재생 요청 위임
	CinematicActionComp->PlayCinematicRequest(Request);
}

void USkillComponent::StopActiveMontages()
{
	AActor* Owner = GetOwner();
	if (!Owner) 
		return;

	ACharacter* Char = Cast<ACharacter>(Owner);
	if (!Char || !Char->GetMesh())
		return;
	
	UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
	if (!AnimInst) return;
	
	// 현재 활성화된 몽타주를 정지
	AnimInst->Montage_Stop(0.1f);
}
