#include "System/Combat/CombatTimeDilationSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UCombatTimeDilationSubsystem::PlayHitStop(AActor* PlayerActor, AActor* TargetActor)
{
	PlayHitStop(DefaultHitStopSettings, PlayerActor, TargetActor);
}

void UCombatTimeDilationSubsystem::PlayHitStop(const FHitStopSettings& Settings, AActor* PlayerActor, AActor* TargetActor)
{
	const float TimeDilation = FMath::Clamp(Settings.TimeDilation, 0.01f, 1.0f);
	const float Duration = FMath::Max(0.0f, Settings.Duration);
	if (Duration <= 0.0f)
	{
		return;
	}

	switch (Settings.TargetMode)
	{
	case EHitStopTargetMode::Global:
	{
		FWorldSlowSettings GlobalHitStopSettings;
		GlobalHitStopSettings.TimeDilation = TimeDilation;
		GlobalHitStopSettings.Duration = Duration;
		PlayWorldSlow(GlobalHitStopSettings);
		break;
	}
	case EHitStopTargetMode::PlayerAndTarget:
		ApplyActorHitStop(PlayerActor, TimeDilation, Duration);
		ApplyActorHitStop(TargetActor, TimeDilation, Duration);
		break;
	case EHitStopTargetMode::TargetOnly:
		ApplyActorHitStop(TargetActor, TimeDilation, Duration);
		break;
	case EHitStopTargetMode::PlayerOnly:
		ApplyActorHitStop(PlayerActor, TimeDilation, Duration);
		break;
	default:
		break;
	}
}

void UCombatTimeDilationSubsystem::PlayWorldSlow()
{
	PlayWorldSlow(DefaultWorldSlowSettings);
}

void UCombatTimeDilationSubsystem::PlayWorldSlow(const FWorldSlowSettings& Settings)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float TimeDilation = FMath::Clamp(Settings.TimeDilation, 0.01f, 1.0f);
	const float Duration = FMath::Max(0.0f, Settings.Duration);
	if (Duration <= 0.0f)
	{
		return;
	}

	if (!bGlobalTimeDilationActive)
	{
		CachedGlobalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(World);
	}

	World->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
	bGlobalTimeDilationActive = true;
	UGameplayStatics::SetGlobalTimeDilation(World, TimeDilation);

	const float GameTimeDuration = Duration * TimeDilation;
	World->GetTimerManager().SetTimer(
		GlobalTimeDilationTimerHandle,
		this,
		&UCombatTimeDilationSubsystem::RestoreGlobalTimeDilation,
		GameTimeDuration,
		false);
}

void UCombatTimeDilationSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GlobalTimeDilationTimerHandle);
		if (bGlobalTimeDilationActive)
		{
			UGameplayStatics::SetGlobalTimeDilation(World, CachedGlobalTimeDilation);
		}

		for (TPair<TWeakObjectPtr<AActor>, FActorTimeDilationRestoreData>& Pair : ActorRestoreDataMap)
		{
			World->GetTimerManager().ClearTimer(Pair.Value.TimerHandle);
			if (AActor* Actor = Pair.Key.Get())
			{
				Actor->CustomTimeDilation = Pair.Value.OriginalTimeDilation;
			}
		}
	}

	ActorRestoreDataMap.Empty();
	Super::Deinitialize();
}

void UCombatTimeDilationSubsystem::ApplyActorHitStop(AActor* Actor, float TimeDilation, float Duration)
{
	if (!Actor || Duration <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TWeakObjectPtr<AActor> ActorKey(Actor);
	FActorTimeDilationRestoreData& RestoreData = ActorRestoreDataMap.FindOrAdd(ActorKey);
	if (!World->GetTimerManager().IsTimerActive(RestoreData.TimerHandle))
	{
		RestoreData.OriginalTimeDilation = Actor->CustomTimeDilation;
	}

	World->GetTimerManager().ClearTimer(RestoreData.TimerHandle);
	Actor->CustomTimeDilation = TimeDilation;

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &UCombatTimeDilationSubsystem::RestoreActorTimeDilation, ActorKey);
	World->GetTimerManager().SetTimer(RestoreData.TimerHandle, TimerDelegate, Duration, false);
}

void UCombatTimeDilationSubsystem::RestoreActorTimeDilation(TWeakObjectPtr<AActor> Actor)
{
	FActorTimeDilationRestoreData RestoreData;
	if (!ActorRestoreDataMap.RemoveAndCopyValue(Actor, RestoreData))
	{
		return;
	}

	if (AActor* ValidActor = Actor.Get())
	{
		ValidActor->CustomTimeDilation = RestoreData.OriginalTimeDilation;
	}
}

void UCombatTimeDilationSubsystem::RestoreGlobalTimeDilation()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, CachedGlobalTimeDilation);
	}

	bGlobalTimeDilationActive = false;
}
