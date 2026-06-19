#include "System/Audio/SoundManagerSubsystem.h"

#include "System/Audio/SoundDataAsset.h"
#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"
#include "FMODBus.h"
#include "FMODEvent.h"
#include "fmod_studio.hpp"
#include "Misc/ConfigCacheIni.h"

DEFINE_LOG_CATEGORY_STATIC(LogSoundManager, Log, All);

namespace SoundManagerConfig
{
	const TCHAR* Section = TEXT("/Script/Project_MJS.Audio");
	const TCHAR* Master = TEXT("Master");
	const TCHAR* BGM = TEXT("BGM");
	const TCHAR* SFX = TEXT("SFX");
	const TCHAR* UI = TEXT("UI");
	const TCHAR* Voice = TEXT("Voice");
	const TCHAR* Ambience = TEXT("Ambience");
	const FName BGMIntensityParameter(TEXT("Intensity"));
}

void USoundManagerSubsystem::Deinitialize()
{
	StopBGM(false);
	StopAmbience(false);
	StopAllAttachedSFX();

	SoundData = nullptr;
	LastPlayTimes.Reset();

	Super::Deinitialize();
}

void USoundManagerSubsystem::SetSoundData(USoundDataAsset* InSoundData)
{
	SoundData = InSoundData;
	LoadVolumeSettings();
}




// ========= BGM ========

void USoundManagerSubsystem::PlayBGM(UFMODEvent* Event, float Volume)
{
	if (!Event)
	{
		return;
	}

	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (bHasBGMInstance && BGMInstance.Instance && CurrentBGMEvent == Event)
	{
		UFMODBlueprintStatics::EventInstanceSetPaused(BGMInstance, false);
		UFMODBlueprintStatics::EventInstanceSetVolume(BGMInstance, ClampedVolume);
		return;
	}

	StopBGM(true);

	StartPersistentInstance(BGMInstance, Event, ClampedVolume);
	if (!BGMInstance.Instance)
	{
		return;
	}

	CurrentBGMEvent = Event;
	bHasBGMInstance = true;

	if (bDebugSound)
	{
		UE_LOG(LogSoundManager, Log, TEXT("PlayBGM: %s"), *GetNameSafe(Event));
	}
}

void USoundManagerSubsystem::StopBGM(bool bAllowFadeOut)
{
	if (!bHasBGMInstance || !BGMInstance.Instance)
	{
		CurrentBGMEvent = nullptr;
		BGMInstance.Instance = nullptr;
		bHasBGMInstance = false;
		return;
	}

	StopPersistentInstance(BGMInstance, bAllowFadeOut);

	CurrentBGMEvent = nullptr;
	bHasBGMInstance = false;
	CurrentBGMState = ESoundBGMState::None;
}

void USoundManagerSubsystem::PauseBGM(bool bPaused)
{
	if (bHasBGMInstance && BGMInstance.Instance)
	{
		UFMODBlueprintStatics::EventInstanceSetPaused(BGMInstance, bPaused);
	}
}

void USoundManagerSubsystem::SetBGMParameter(FName ParameterName, float Value)
{
	if (bHasBGMInstance && BGMInstance.Instance && !ParameterName.IsNone())
	{
		UFMODBlueprintStatics::EventInstanceSetParameter(BGMInstance, ParameterName, Value);
	}
}

void USoundManagerSubsystem::SetBGMState(ESoundBGMState NewState, float Volume)
{
	if (CurrentBGMState == NewState && bHasBGMInstance && BGMInstance.Instance)
	{
		return;
	}

	UFMODEvent* Event = GetBGMEventForState(NewState);
	if (!Event)
	{
		StopBGM(true);
		CurrentBGMState = NewState;
		return;
	}

	CurrentBGMState = NewState;
	PlayBGM(Event, Volume);
	CurrentBGMState = NewState;
}

void USoundManagerSubsystem::SetBGMIntensity(float Intensity)
{
	SetBGMParameter(SoundManagerConfig::BGMIntensityParameter, FMath::Clamp(Intensity, 0.0f, 1.0f));
}







// ========= SFX ========

void USoundManagerSubsystem::PlaySFX2D(UFMODEvent* Event, float Volume, float Cooldown)
{
	if (!Event || !CanPlayEvent(Event, Cooldown))
	{
		return;
	}

	FFMODEventInstance Instance = UFMODBlueprintStatics::PlayEvent2D(this, Event, false);
	if (!Instance.Instance)
	{
		return;
	}

	UFMODBlueprintStatics::EventInstanceSetVolume(Instance, FMath::Clamp(Volume, 0.0f, 1.0f));
	UFMODBlueprintStatics::EventInstancePlay(Instance);

	if (bDebugSound)
	{
		UE_LOG(LogSoundManager, Verbose, TEXT("PlaySFX2D: %s"), *GetNameSafe(Event));
	}
}

void USoundManagerSubsystem::PlaySFXAtLocation(UFMODEvent* Event, FVector Location, float Volume, float Cooldown)
{
	if (!Event || !CanPlayEvent(Event, Cooldown))
	{
		return;
	}

	const FTransform Transform(FRotator::ZeroRotator, Location);
	FFMODEventInstance Instance = UFMODBlueprintStatics::PlayEventAtLocation(this, Event, Transform, false);
	if (!Instance.Instance)
	{
		return;
	}

	UFMODBlueprintStatics::EventInstanceSetVolume(Instance, FMath::Clamp(Volume, 0.0f, 1.0f));
	UFMODBlueprintStatics::EventInstancePlay(Instance);

	if (bDebugSound)
	{
		UE_LOG(LogSoundManager, Verbose, TEXT("PlaySFXAtLocation: %s Location=%s"), *GetNameSafe(Event), *Location.ToCompactString());
	}
}

UFMODAudioComponent* USoundManagerSubsystem::PlaySFXAttached(
	FName Handle,
	UFMODEvent* Event,
	USceneComponent* AttachToComponent,
	FName AttachPointName,
	float Volume)
{
	if (Handle.IsNone() || !Event || !AttachToComponent)
	{
		return nullptr;
	}

	StopAttachedSFX(Handle);

	UFMODAudioComponent* AudioComponent = UFMODBlueprintStatics::PlayEventAttached(
		Event,
		AttachToComponent,
		AttachPointName,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		true,
		false,
		false);

	if (!AudioComponent)
	{
		return nullptr;
	}

	AudioComponent->SetVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
	AudioComponent->Play();

	AttachedSFXMap.Add(Handle, AudioComponent);

	if (bDebugSound)
	{
		UE_LOG(LogSoundManager, Verbose, TEXT("PlaySFXAttached: Handle=%s Event=%s"), *Handle.ToString(), *GetNameSafe(Event));
	}

	return AudioComponent;
}

void USoundManagerSubsystem::StopAttachedSFX(FName Handle)
{
	if (Handle.IsNone())
	{
		return;
	}

	TObjectPtr<UFMODAudioComponent>* FoundComponent = AttachedSFXMap.Find(Handle);
	if (!FoundComponent)
	{
		return;
	}

	if (*FoundComponent)
	{
		(*FoundComponent)->Stop();
		(*FoundComponent)->DestroyComponent();
	}

	AttachedSFXMap.Remove(Handle);
}






// ========= Ambience ========

void USoundManagerSubsystem::PlayAmbience(UFMODEvent* Event, float Volume)
{
	if (!Event)
	{
		return;
	}

	const float ClampedVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	if (bHasAmbienceInstance && AmbienceInstance.Instance && CurrentAmbienceEvent == Event)
	{
		UFMODBlueprintStatics::EventInstanceSetPaused(AmbienceInstance, false);
		UFMODBlueprintStatics::EventInstanceSetVolume(AmbienceInstance, ClampedVolume);
		return;
	}

	StopAmbience(true);

	StartPersistentInstance(AmbienceInstance, Event, ClampedVolume);
	if (!AmbienceInstance.Instance)
	{
		return;
	}

	CurrentAmbienceEvent = Event;
	bHasAmbienceInstance = true;

	if (bDebugSound)
	{
		UE_LOG(LogSoundManager, Log, TEXT("PlayAmbience: %s"), *GetNameSafe(Event));
	}
}

void USoundManagerSubsystem::StopAmbience(bool bAllowFadeOut)
{
	if (!bHasAmbienceInstance || !AmbienceInstance.Instance)
	{
		CurrentAmbienceEvent = nullptr;
		AmbienceInstance.Instance = nullptr;
		bHasAmbienceInstance = false;
		return;
	}

	StopPersistentInstance(AmbienceInstance, bAllowFadeOut);

	CurrentAmbienceEvent = nullptr;
	bHasAmbienceInstance = false;
}

void USoundManagerSubsystem::PauseAmbience(bool bPaused)
{
	if (bHasAmbienceInstance && AmbienceInstance.Instance)
	{
		UFMODBlueprintStatics::EventInstanceSetPaused(AmbienceInstance, bPaused);
	}
}

void USoundManagerSubsystem::SetAmbienceParameter(FName ParameterName, float Value)
{
	if (bHasAmbienceInstance && AmbienceInstance.Instance && !ParameterName.IsNone())
	{
		UFMODBlueprintStatics::EventInstanceSetParameter(AmbienceInstance, ParameterName, Value);
	}
}







// ========= Bus / Mix ========

void USoundManagerSubsystem::ApplyVolumes(
	float MasterVolume,
	float BGMVolume,
	float SFXVolume,
	float UIVolume,
	float VoiceVolume,
	float AmbienceVolume)
{
	if (!SoundData)
	{
		return;
	}

	CurrentMasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	CurrentBGMVolume = FMath::Clamp(BGMVolume, 0.0f, 1.0f);
	CurrentSFXVolume = FMath::Clamp(SFXVolume, 0.0f, 1.0f);
	CurrentUIVolume = FMath::Clamp(UIVolume, 0.0f, 1.0f);
	CurrentVoiceVolume = FMath::Clamp(VoiceVolume, 0.0f, 1.0f);
	CurrentAmbienceVolume = FMath::Clamp(AmbienceVolume, 0.0f, 1.0f);

	SetBusVolume(SoundData->Buses.Master, CurrentMasterVolume);
	SetBusVolume(SoundData->Buses.BGM, CurrentBGMVolume);
	SetBusVolume(SoundData->Buses.SFX, CurrentSFXVolume);
	SetBusVolume(SoundData->Buses.UI, CurrentUIVolume);
	SetBusVolume(SoundData->Buses.Voice, CurrentVoiceVolume);
	SetBusVolume(SoundData->Buses.Ambience, CurrentAmbienceVolume);
}

void USoundManagerSubsystem::SetSFXPaused(bool bPaused)
{
	if (!SoundData)
	{
		return;
	}

	SetBusPaused(SoundData->Buses.SFX, bPaused);
	SetBusPaused(SoundData->Buses.Ambience, bPaused);
}

void USoundManagerSubsystem::LoadVolumeSettings()
{
	float Value = 1.0f;

	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::Master, Value, GGameUserSettingsIni))
	{
		CurrentMasterVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::BGM, Value, GGameUserSettingsIni))
	{
		CurrentBGMVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::SFX, Value, GGameUserSettingsIni))
	{
		CurrentSFXVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::UI, Value, GGameUserSettingsIni))
	{
		CurrentUIVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::Voice, Value, GGameUserSettingsIni))
	{
		CurrentVoiceVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}
	if (GConfig->GetFloat(SoundManagerConfig::Section, SoundManagerConfig::Ambience, Value, GGameUserSettingsIni))
	{
		CurrentAmbienceVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	}

	ApplyVolumes(CurrentMasterVolume, CurrentBGMVolume, CurrentSFXVolume, CurrentUIVolume, CurrentVoiceVolume, CurrentAmbienceVolume);
}

void USoundManagerSubsystem::SaveVolumeSettings() const
{
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::Master, CurrentMasterVolume, GGameUserSettingsIni);
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::BGM, CurrentBGMVolume, GGameUserSettingsIni);
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::SFX, CurrentSFXVolume, GGameUserSettingsIni);
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::UI, CurrentUIVolume, GGameUserSettingsIni);
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::Voice, CurrentVoiceVolume, GGameUserSettingsIni);
	GConfig->SetFloat(SoundManagerConfig::Section, SoundManagerConfig::Ambience, CurrentAmbienceVolume, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void USoundManagerSubsystem::ApplyPauseAudio(float PauseBGMVolume)
{
	if (bPauseAudioApplied)
	{
		return;
	}

	PrePauseBGMVolume = CurrentBGMVolume;
	SetSFXPaused(true);
	SetBusVolume(SoundData ? SoundData->Buses.BGM : nullptr, FMath::Clamp(PauseBGMVolume, 0.0f, 1.0f));
	bPauseAudioApplied = true;
}

void USoundManagerSubsystem::RestorePauseAudio()
{
	if (!bPauseAudioApplied)
	{
		return;
	}

	SetSFXPaused(false);
	SetBusVolume(SoundData ? SoundData->Buses.BGM : nullptr, PrePauseBGMVolume);
	bPauseAudioApplied = false;
}

void USoundManagerSubsystem::SetDebugSoundEnabled(bool bEnabled)
{
	bDebugSound = bEnabled;
}

void USoundManagerSubsystem::DumpActiveSounds() const
{
	UE_LOG(LogSoundManager, Log, TEXT("Sound dump: BGM=%s State=%d Ambience=%s AttachedLoops=%d"),
		*GetNameSafe(CurrentBGMEvent),
		static_cast<int32>(CurrentBGMState),
		*GetNameSafe(CurrentAmbienceEvent),
		AttachedSFXMap.Num());

	for (const TPair<FName, TObjectPtr<UFMODAudioComponent>>& Pair : AttachedSFXMap)
	{
		UE_LOG(LogSoundManager, Log, TEXT("  Attached: %s Component=%s"), *Pair.Key.ToString(), *GetNameSafe(Pair.Value));
	}
}

bool USoundManagerSubsystem::CanPlayEvent(UFMODEvent* Event, float Cooldown)
{
	if (!Event || Cooldown <= 0.0f)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const float CurrentTime = World->GetTimeSeconds();
	const TObjectKey<UFMODEvent> EventKey(Event);
	const float* LastPlayTime = LastPlayTimes.Find(EventKey);
	if (LastPlayTime && CurrentTime - *LastPlayTime < Cooldown)
	{
		return false;
	}

	LastPlayTimes.Add(EventKey, CurrentTime);
	return true;
}

UFMODEvent* USoundManagerSubsystem::GetBGMEventForState(ESoundBGMState State) const
{
	if (!SoundData)
	{
		return nullptr;
	}

	switch (State)
	{
	case ESoundBGMState::Exploration:
		return SoundData->Music.GameplayBGM;
	case ESoundBGMState::Combat:
		return SoundData->Music.BattleBGM;
	case ESoundBGMState::Boss:
		return SoundData->Music.BossBGM;
	case ESoundBGMState::Victory:
		return SoundData->Music.VictoryBGM;
	case ESoundBGMState::Death:
		return SoundData->Music.DeathBGM;
	case ESoundBGMState::None:
	default:
		return nullptr;
	}
}

void USoundManagerSubsystem::StartPersistentInstance(FFMODEventInstance& Instance, UFMODEvent* Event, float Volume)
{
	Instance = UFMODBlueprintStatics::PlayEvent2D(this, Event, false);
	if (!Instance.Instance)
	{
		return;
	}

	Instance.Instance->setVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
	Instance.Instance->start();
}

void USoundManagerSubsystem::StopPersistentInstance(FFMODEventInstance& Instance, bool bAllowFadeOut)
{
	if (!Instance.Instance)
	{
		return;
	}

	const FMOD_STUDIO_STOP_MODE StopMode = bAllowFadeOut
		? FMOD_STUDIO_STOP_ALLOWFADEOUT
		: FMOD_STUDIO_STOP_IMMEDIATE;

	Instance.Instance->stop(StopMode);
	Instance.Instance->release();
	Instance.Instance = nullptr;
}

void USoundManagerSubsystem::SetBusVolume(UFMODBus* Bus, float Volume)
{
	if (Bus)
	{
		UFMODBlueprintStatics::BusSetVolume(Bus, FMath::Clamp(Volume, 0.0f, 1.0f));
	}
}

void USoundManagerSubsystem::SetBusPaused(UFMODBus* Bus, bool bPaused)
{
	if (Bus)
	{
		UFMODBlueprintStatics::BusSetPaused(Bus, bPaused);
	}
}

void USoundManagerSubsystem::StopAllAttachedSFX()
{
	for (TPair<FName, TObjectPtr<UFMODAudioComponent>>& Pair : AttachedSFXMap)
	{
		if (Pair.Value)
		{
			Pair.Value->Stop();
			Pair.Value->DestroyComponent();
		}
	}

	AttachedSFXMap.Reset();
}
