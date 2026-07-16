#include "Cinematic/Director/CinematicPostActionExecutor.h"

#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void FCinematicPostActionExecutor::BeginPlayback(UWorld* World, const FCinematicPostActionConfig& Config, const FTimerDelegate& DuringPlaybackDelegate)
{
	Cancel(World);
	ActiveConfig = Config;

	if (!ActiveConfig.bLoadLevelDuringPlayback)
	{
		return;
	}

	if (ActiveConfig.LevelName.IsNone())
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("Cinematic during-playback level load skipped: LevelName is missing."));
		return;
	}

	if (!World)
	{
		return;
	}

	const float Delay = FMath::Max(0.0f, ActiveConfig.LoadLevelTriggerTime);
	World->GetTimerManager().SetTimer(PlaybackLevelLoadTimerHandle, DuringPlaybackDelegate, Delay, false);
}

void FCinematicPostActionExecutor::Cancel(UWorld* World)
{
	ClearPlaybackLevelLoadTimer(World);
	ClearPendingPostActionTimer(World);
	ActiveConfig = FCinematicPostActionConfig();
	PendingConfig = FCinematicPostActionConfig();
}

bool FCinematicPostActionExecutor::ConsumeDuringPlayback(UWorld* World, FCinematicPostActionConfig& OutConfig)
{
	ClearPlaybackLevelLoadTimer(World);

	if (!ActiveConfig.bLoadLevelDuringPlayback || ActiveConfig.LevelName.IsNone())
	{
		return false;
	}

	OutConfig = ActiveConfig;
	return true;
}

void FCinematicPostActionExecutor::ExecuteOnNaturalFinish(UWorld* World, const FCinematicPostActionConfig& Config, const FTimerDelegate& DelayedDelegate)
{
	if (!Config.bLoadLevelOnFinish || Config.LevelName.IsNone())
	{
		return;
	}

	ClearPendingPostActionTimer(World);

	const float Delay = FMath::Max(0.0f, Config.DelayBeforeLoad);
	if (Delay <= 0.0f)
	{
		ExecuteLevelLoad(World, Config, TEXT("OnFinish"));
		return;
	}

	if (!World)
	{
		return;
	}

	PendingConfig = Config;
	World->GetTimerManager().SetTimer(PendingPostActionTimerHandle, DelayedDelegate, Delay, false);
}

bool FCinematicPostActionExecutor::ConsumePendingPostAction(UWorld* World, FCinematicPostActionConfig& OutConfig)
{
	ClearPendingPostActionTimer(World);

	if (!PendingConfig.bLoadLevelOnFinish || PendingConfig.LevelName.IsNone())
	{
		return false;
	}

	OutConfig = PendingConfig;
	PendingConfig = FCinematicPostActionConfig();
	return true;
}

void FCinematicPostActionExecutor::ExecuteLevelLoad(UWorld* World, const FCinematicPostActionConfig& Config, const TCHAR* ContextLabel)
{
	if ((!Config.bLoadLevelOnFinish && !Config.bLoadLevelDuringPlayback) || Config.LevelName.IsNone())
	{
		return;
	}

	if (!World)
	{
		UE_LOG(LogCinematicSystem, Warning, TEXT("ExecuteLevelLoad failed: World is missing. Context=%s"), ContextLabel);
		return;
	}

	UE_LOG(LogCinematicSystem, Log,
		TEXT("ExecuteLevelLoad: Level=%s Async=%d Context=%s"),
		*Config.LevelName.ToString(),
		Config.bAsyncLoad ? 1 : 0,
		ContextLabel);

	if (Config.bAsyncLoad)
	{
		if (ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(World, Config.LevelName))
		{
			StreamingLevel->SetShouldBeLoaded(true);
			StreamingLevel->SetShouldBeVisible(true);
		}
		else
		{
			UE_LOG(LogCinematicSystem, Warning,
				TEXT("ExecuteLevelLoad: streaming level not found. Level=%s Context=%s"),
				*Config.LevelName.ToString(),
				ContextLabel);
		}
	}
	else
	{
		UGameplayStatics::OpenLevel(World, Config.LevelName);
	}
}

void FCinematicPostActionExecutor::ClearPlaybackLevelLoadTimer(UWorld* World)
{
	if (World && PlaybackLevelLoadTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(PlaybackLevelLoadTimerHandle);
	}

	PlaybackLevelLoadTimerHandle.Invalidate();
}

void FCinematicPostActionExecutor::ClearPendingPostActionTimer(UWorld* World)
{
	if (World && PendingPostActionTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(PendingPostActionTimerHandle);
	}

	PendingPostActionTimerHandle.Invalidate();
}
