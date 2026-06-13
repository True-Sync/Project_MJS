#include "Audio/SoundManagerSubsystem.h"

#include "Audio/SoundDataAsset.h"
#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"
#include "FMODBus.h"
#include "FMODEvent.h"
#include "fmod_studio.hpp"

void USoundManagerSubsystem::Deinitialize()
{
	StopBGM(false);
	StopAllAttachedSFX();

	SoundData = nullptr;
	LastPlayTimes.Reset();

	Super::Deinitialize();
}

void USoundManagerSubsystem::SetSoundData(USoundDataAsset* InSoundData)
{
	SoundData = InSoundData;
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

	BGMInstance = UFMODBlueprintStatics::PlayEvent2D(this, Event, false);
	if (!BGMInstance.Instance)
	{
		return;
	}

	CurrentBGMEvent = Event;
	bHasBGMInstance = true;

	UFMODBlueprintStatics::EventInstanceSetVolume(BGMInstance, ClampedVolume);
	UFMODBlueprintStatics::EventInstancePlay(BGMInstance);
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

	const FMOD_STUDIO_STOP_MODE StopMode = bAllowFadeOut
		? FMOD_STUDIO_STOP_ALLOWFADEOUT
		: FMOD_STUDIO_STOP_IMMEDIATE;

	BGMInstance.Instance->stop(StopMode);
	BGMInstance.Instance->release();

	CurrentBGMEvent = nullptr;
	BGMInstance.Instance = nullptr;
	bHasBGMInstance = false;
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

	SetBusVolume(SoundData->Buses.Master, MasterVolume);
	SetBusVolume(SoundData->Buses.BGM, BGMVolume);
	SetBusVolume(SoundData->Buses.SFX, SFXVolume);
	SetBusVolume(SoundData->Buses.UI, UIVolume);
	SetBusVolume(SoundData->Buses.Voice, VoiceVolume);
	SetBusVolume(SoundData->Buses.Ambience, AmbienceVolume);
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
