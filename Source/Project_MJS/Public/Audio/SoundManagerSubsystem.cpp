#include "Audio/SoundManagerSubsystem.h"
#include "FMODEvent.h"
#include "FMODBus.h"
#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"
#include "Audio/SoundDataAsset.h"

void USoundManagerSubsystem::Deinitialize()
{
	StopBGM(false);
	StopAllAttachedSFX();	
	
	SoundData = nullptr;
	
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
		return;
	
	
	if (bHasBGMInstance && BGMInstance.Instance && CurrentBGMEvent == Event)
	{
		UFMODBlueprintStatics::EventInstanceSetPaused(BGMInstance, false);
		UFMODBlueprintStatics::EventInstanceSetVolume(BGMInstance, FMath::Clamp(Volume, 0.0f, 1.0f));
		
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
	
	UFMODBlueprintStatics::EventInstanceSetVolume(BGMInstance, FMath::Clamp(Volume, 0.0f, 1.0f));
	UFMODBlueprintStatics::EventInstancePlay(BGMInstance);
}

void USoundManagerSubsystem::StopBGM(bool bAllowFadeOut)
{
	if (!bHasBGMInstance && BGMInstance.Instance)
	{
		CurrentBGMEvent = nullptr;
		bHasBGMInstance = false;
		
		return;
	}
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

void USoundManagerSubsystem::PlaySFX2D(UFMODEvent* Event, float Volume)
{
	if (!Event)
		return;
	
	FFMODEventInstance Instance = UFMODBlueprintStatics::PlayEvent2D(this, Event, false);
	if (!Instance.Instance)
		return;
	
	UFMODBlueprintStatics::EventInstanceSetVolume(Instance, FMath::Clamp(Volume, 0.0f, 1.0f));
	UFMODBlueprintStatics::EventInstancePlay(Instance);
}

void USoundManagerSubsystem::PlaySFXAtLocation(UFMODEvent* Event, FVector Location, float Volume)
{
	if (!Event)
		return;
	const FTransform Transform(FRotator::ZeroRotator, Location);

	FFMODEventInstance Instance = UFMODBlueprintStatics::PlayEventAtLocation(this, Event, Transform, false);
	if (!Instance.Instance)
		return;
	
	UFMODBlueprintStatics::EventInstanceSetVolume(Instance, FMath::Clamp(Volume, 0.0f, 1.0f));
	UFMODBlueprintStatics::EventInstancePlay(Instance);
}

UFMODAudioComponent* USoundManagerSubsystem::PlaySFXAttached(FName Handle, UFMODEvent* Event,
	USceneComponent* AttachToComponent, FName AttachPointName, float Volume)
{
	if (Handle.IsNone() || !Event || !AttachToComponent)
		return nullptr;
	
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
		return nullptr;
	
	AudioComponent->SetVolume(FMath::Clamp(Volume, 0.0f, 1.0f));
	AudioComponent->Play();

	AttachedSFXMap.Add(Handle, AudioComponent);
	return AudioComponent;
}

void USoundManagerSubsystem::StopAttachedSFX(FName Handle)
{
	if (Handle.IsNone())
		return;

	TObjectPtr<UFMODAudioComponent>* FoundComponent = AttachedSFXMap.Find(Handle);
	if (!FoundComponent)
		return;
	
	if (*FoundComponent)
	{
		(*FoundComponent)->Stop();
		(*FoundComponent)->DestroyComponent();
	}

	AttachedSFXMap.Remove(Handle);
}







// ========= Bus / Mix ========

void USoundManagerSubsystem::ApplyVolumes(float MasterVolume, float BGMVolume, float SFXVolume, float UIVolume)
{
	if (!SoundData)
		return;
	
	SetBusVolume(SoundData->Buses.Master, MasterVolume);
	SetBusVolume(SoundData->Buses.BGM, BGMVolume);
	SetBusVolume(SoundData->Buses.SFX, SFXVolume);
	SetBusVolume(SoundData->Buses.UI, UIVolume);
}

void USoundManagerSubsystem::SetBusVolume(UFMODBus* Bus, float Volume)
{
	if (!Bus)
		return;
	
	UFMODBlueprintStatics::BusSetVolume(Bus, FMath::Clamp(Volume, 0.0f, 1.0f));
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
