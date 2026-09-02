#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "Character/SharedData/StaminaCostData.h"
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

USTRUCT(BlueprintType)
struct PROJECT_MJS_API FSkillExecutionSettings
{
	GENERATED_BODY()

	// 스킬 발동 시 재생할 애니메이션 몽타주.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	// 이 스킬이 소모하는 스태미나 비용 (UStaminaComponent와 연동)
	// 스태미나 관련 규칙은 StaminaComponent + FStaminaCostData에서 통일 관리.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Resource")
	FStaminaCostData StaminaCost;
};

USTRUCT(BlueprintType)
struct PROJECT_MJS_API FSkillCinematicSettings
{
	GENERATED_BODY()

	// Level Sequence 기반 시네마틱을 사용할지 여부.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Cinematic")
	bool bUseCinematic = true;

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

UCLASS(BlueprintType, Blueprintable)
class PROJECT_MJS_API USkillDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual void PostLoad() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	FText SkillName;

	// 연출 타입. 일반 스킬과 궁극기의 시네마틱 기본 동작을 나누는 데 사용한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Info")
	ESkillType Type = ESkillType::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Settings")
	FSkillExecutionSettings ExecutionSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill|Settings")
	FSkillCinematicSettings CinematicSettings;

private:
	// 구조체 분리 이전에 저장된 DataAsset 값을 한 번 마이그레이션하기 위한 레거시 필드.
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use ExecutionSettings.MontageToPlay instead."))
	TObjectPtr<UAnimMontage> MontageToPlay = nullptr;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use ExecutionSettings.StaminaCost instead."))
	FStaminaCostData StaminaCost;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CinematicSettings.bUseCinematic instead."))
	bool bUseCinematic = true;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use CinematicSettings.CinematicSequence instead."))
	TObjectPtr<ULevelSequence> CinematicSequence = nullptr;

	UPROPERTY(meta = (DeprecatedProperty))
	float CinematicBlendOutTime = 0.15f;

	UPROPERTY(meta = (DeprecatedProperty))
	bool bRestoreViewTarget = true;

	UPROPERTY(meta = (DeprecatedProperty))
	bool bStopPreviousCinematic = false;

	UPROPERTY(meta = (DeprecatedProperty))
	bool bUseUltimateCinematicDefaults = true;

	UPROPERTY(meta = (DeprecatedProperty))
	ECinematicParticipantScope ParticipantScope = ECinematicParticipantScope::ExplicitOnly;

	UPROPERTY(meta = (DeprecatedProperty))
	ECinematicAnchorMode AnchorMode = ECinematicAnchorMode::InstigatorToSubject;

	UPROPERTY(meta = (DeprecatedProperty))
	ECinematicRotationSource RotationSource = ECinematicRotationSource::AnchorTransform;

	UPROPERTY(meta = (DeprecatedProperty))
	FName AnchorSocketName = NAME_None;

	UPROPERTY(meta = (DeprecatedProperty))
	FName TargetSocketName = NAME_None;

	UPROPERTY(meta = (DeprecatedProperty))
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(meta = (DeprecatedProperty))
	bool bUseYawOnly = true;

	UPROPERTY(meta = (DeprecatedProperty))
	bool bBindBestTarget = true;

	UPROPERTY(meta = (DeprecatedProperty))
	FName TargetBindingTag = TEXT("Target");

	UPROPERTY(meta = (DeprecatedProperty))
	bool bAllowTargetBindingFromAsset = false;
};
