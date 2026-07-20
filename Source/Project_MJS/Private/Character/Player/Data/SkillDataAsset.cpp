#include "Character/Player/Data/SkillDataAsset.h"

void USkillDataAsset::PostLoad()
{
	Super::PostLoad();

	// 구조체 분리 이전 에셋은 몽타주 또는 시퀀스 참조를 기준으로 기존 값을 새 설정 구조체에 옮긴다.
	const bool bHasLegacyAssetReferences = MontageToPlay != nullptr || CinematicSequence != nullptr;
	const bool bHasMigratedAssetReferences = ExecutionSettings.MontageToPlay != nullptr || CinematicSettings.CinematicSequence != nullptr;
	if (!bHasLegacyAssetReferences || bHasMigratedAssetReferences)
	{
		return;
	}

	ExecutionSettings.MontageToPlay = MontageToPlay;
	ExecutionSettings.StaminaCost = StaminaCost;

	CinematicSettings.bUseCinematic = bUseCinematic;
	CinematicSettings.CinematicSequence = CinematicSequence;
	CinematicSettings.CinematicBlendOutTime = CinematicBlendOutTime;
	CinematicSettings.bRestoreViewTarget = bRestoreViewTarget;
	CinematicSettings.bStopPreviousCinematic = bStopPreviousCinematic;
	CinematicSettings.bUseUltimateCinematicDefaults = bUseUltimateCinematicDefaults;
	CinematicSettings.ParticipantScope = ParticipantScope;
	CinematicSettings.AnchorMode = AnchorMode;
	CinematicSettings.RotationSource = RotationSource;
	CinematicSettings.AnchorSocketName = AnchorSocketName;
	CinematicSettings.TargetSocketName = TargetSocketName;
	CinematicSettings.RelativeTransform = RelativeTransform;
	CinematicSettings.bUseYawOnly = bUseYawOnly;
	CinematicSettings.bBindBestTarget = bBindBestTarget;
	CinematicSettings.TargetBindingTag = TargetBindingTag;
	CinematicSettings.bAllowTargetBindingFromAsset = bAllowTargetBindingFromAsset;

	// 마이그레이션된 에셋을 저장한 뒤 레거시 값이 다시 덮어쓰지 않도록 참조를 비운다.
	MontageToPlay = nullptr;
	CinematicSequence = nullptr;
}
