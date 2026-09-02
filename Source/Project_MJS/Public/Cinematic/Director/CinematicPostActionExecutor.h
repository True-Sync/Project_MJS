#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "TimerManager.h"
#include "CinematicPostActionExecutor.generated.h"

class UWorld;

/** Owns cinematic post-action timers and executes the requested level transition. */
USTRUCT()
struct PROJECT_MJS_API FCinematicPostActionExecutor
{
	GENERATED_BODY()

public:
	void BeginPlayback(UWorld* World, const FCinematicPostActionConfig& Config, const FTimerDelegate& DuringPlaybackDelegate);
	void Cancel(UWorld* World);
	const FCinematicPostActionConfig& GetActiveConfig() const { return ActiveConfig; }
	bool ConsumeDuringPlayback(UWorld* World, FCinematicPostActionConfig& OutConfig);
	void ExecuteOnNaturalFinish(UWorld* World, const FCinematicPostActionConfig& Config, const FTimerDelegate& DelayedDelegate);
	bool ConsumePendingPostAction(UWorld* World, FCinematicPostActionConfig& OutConfig);

	static void ExecuteLevelLoad(UWorld* World, const FCinematicPostActionConfig& Config, const TCHAR* ContextLabel);

private:
	void ClearPlaybackLevelLoadTimer(UWorld* World);
	void ClearPendingPostActionTimer(UWorld* World);

	UPROPERTY(Transient)
	FCinematicPostActionConfig ActiveConfig;

	UPROPERTY(Transient)
	FCinematicPostActionConfig PendingConfig;

	FTimerHandle PlaybackLevelLoadTimerHandle;
	FTimerHandle PendingPostActionTimerHandle;
};
