#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "CombatTimeDilationSubsystem.generated.h"

UENUM(BlueprintType)
enum class ECombatTimeDilationFeedbackMode : uint8
{
	HitStop,
	WorldSlow
};

UENUM(BlueprintType)
enum class EHitStopTargetMode : uint8
{
	Global,
	PlayerAndTarget,
	TargetOnly,
	PlayerOnly
};

USTRUCT(BlueprintType)
struct FHitStopSettings
{
	GENERATED_BODY()

	// 히트스톱을 적용할 대상 범위
	UPROPERTY(EditAnywhere, Category = "HitStop")
	EHitStopTargetMode TargetMode = EHitStopTargetMode::PlayerAndTarget;

	// 히트스톱 중 적용할 시간 배율
	UPROPERTY(EditAnywhere, Category = "HitStop", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TimeDilation = 0.05f;

	// 히트스톱 유지 시간
	UPROPERTY(EditAnywhere, Category = "HitStop", meta = (ClampMin = "0.0"))
	float Duration = 0.08f;
};

USTRUCT(BlueprintType)
struct FWorldSlowSettings
{
	GENERATED_BODY()

	// 월드 슬로우 중 적용할 전역 시간 배율
	UPROPERTY(EditAnywhere, Category = "WorldSlow", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float TimeDilation = 0.25f;

	// 월드 슬로우 유지 시간
	UPROPERTY(EditAnywhere, Category = "WorldSlow", meta = (ClampMin = "0.0"))
	float Duration = 0.18f;
};

UCLASS()
class PROJECT_MJS_API UCombatTimeDilationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void PlayHitStop(AActor* PlayerActor, AActor* TargetActor);
	void PlayHitStop(const FHitStopSettings& Settings, AActor* PlayerActor, AActor* TargetActor);

	void PlayWorldSlow();
	void PlayWorldSlow(const FWorldSlowSettings& Settings);

	virtual void Deinitialize() override;

private:
	struct FActorTimeDilationRestoreData
	{
		// 히트스톱 적용 전 액터 시간 배율
		float OriginalTimeDilation = 1.0f;

		// 액터 시간 배율 복구 타이머
		FTimerHandle TimerHandle;
	};

	// 단일 액터에 히트스톱을 적용하고 복구 타이머 등록
	void ApplyActorHitStop(AActor* Actor, float TimeDilation, float Duration);

	// 히트스톱이 끝난 액터의 시간 배율 복구
	void RestoreActorTimeDilation(TWeakObjectPtr<AActor> Actor);

	// 월드 슬로우가 끝난 뒤 전역 시간 배율 복구
	void RestoreGlobalTimeDilation();

	// 인자 없는 히트스톱 호출에서 사용할 기본 설정
	FHitStopSettings DefaultHitStopSettings;

	// 인자 없는 월드 슬로우 호출에서 사용할 기본 설정
	FWorldSlowSettings DefaultWorldSlowSettings;

	// 현재 히트스톱이 적용 중인 액터와 복구 정보
	TMap<TWeakObjectPtr<AActor>, FActorTimeDilationRestoreData> ActorRestoreDataMap;

	// 월드 슬로우 적용 전 전역 시간 배율
	float CachedGlobalTimeDilation = 1.0f;

	// 월드 슬로우가 현재 적용 중인지 여부
	bool bGlobalTimeDilationActive = false;

	// 전역 시간 배율 복구 타이머
	FTimerHandle GlobalTimeDilationTimerHandle;
};
