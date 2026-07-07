#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "Engine/DataAsset.h"
#include "SkillDataAsset.generated.h"

class UAnimMontage;
class ULevelSequence;

// 스킬 연출 타입 구분
UENUM(BlueprintType)
enum class ESkillType : uint8
{
	// 일반 스킬
	Normal UMETA(DisplayName = "Normal"),
	// 궁극기
	Ultimate UMETA(DisplayName = "Ultimate"),
};

UCLASS(BlueprintType, Blueprintable)
class PROJECT_MJS_API USkillDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	FText SkillName;

	// 연출 타입. 일반 스킬과 궁극기의 시네마틱 기본 동작을 나누는 데 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	ESkillType Type = ESkillType::Normal;

	// Level Sequence 기반 시네마틱을 사용할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic")
	bool bUseCinematic = true;

	// 스킬 발동 시 재생할 애니메이션 몽타주.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	// 스킬 발동 시 재생할 Level Sequence.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	TObjectPtr<ULevelSequence> CinematicSequence = nullptr;

	// 시네마틱 종료 후 기존 ViewTarget으로 돌아갈 때의 블렌드 시간.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic", ClampMin = "0.0"))
	float CinematicBlendOutTime = 0.15f;

	// 시네마틱 종료 후 발동 전 ViewTarget으로 복구할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	bool bRestoreViewTarget = true;

	// 다른 시네마틱이 이미 재생 중이면 중단하고 이 스킬 시네마틱을 재생할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	bool bStopPreviousCinematic = false;

	// 궁극기일 때 전역 참가자, 기존 컷신 중단 같은 기본값을 자동 적용할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	bool bUseUltimateCinematicDefaults = true;

	// 시네마틱 참가자를 명시 대상만 모을지, 월드 전체에서 모을지 결정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic", meta = (EditCondition = "bUseCinematic"))
	ECinematicParticipantScope ParticipantScope = ECinematicParticipantScope::ExplicitOnly;

	// 시퀀스 Transform Origin을 어디에 맞출지 결정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic"))
	ECinematicAnchorMode AnchorMode = ECinematicAnchorMode::InstigatorToSubject;

	// 최종 시퀀스 회전값을 어디에서 가져올지 결정한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic"))
	ECinematicRotationSource RotationSource = ECinematicRotationSource::AnchorTransform;

	// 시퀀스 원점을 잡을 때 사용할 발동자 소켓.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic"))
	FName AnchorSocketName = NAME_None;

	// InstigatorToSubject 모드에서 바라볼 대상 소켓.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic"))
	FName TargetSocketName = NAME_None;

	// 계산된 시퀀스 원점에 추가로 적용할 로컬 오프셋.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic", MakeEditWidget = "true"))
	FTransform RelativeTransform = FTransform::Identity;

	// 캐릭터 액션 게임용으로 Pitch/Roll을 제거하고 Yaw만 사용할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Dynamic Transform", meta = (EditCondition = "bUseCinematic"))
	bool bUseYawOnly = true;

	// 타겟팅 컴포넌트의 BestTarget을 시퀀스 바인딩에 넘길지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Binding", meta = (EditCondition = "bUseCinematic"))
	bool bBindBestTarget = true;

	// Level Sequence에서 대상 Actor Binding에 지정할 태그 이름.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Binding", meta = (EditCondition = "bUseCinematic && bBindBestTarget"))
	FName TargetBindingTag = TEXT("Target");

	// true면 시퀀스 에셋의 기존 바인딩을 유지하고 대상 바인딩을 추가한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic|Binding", meta = (EditCondition = "bUseCinematic && bBindBestTarget"))
	bool bAllowTargetBindingFromAsset = false;
};
