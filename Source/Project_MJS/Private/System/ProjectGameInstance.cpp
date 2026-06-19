#include "System/ProjectGameInstance.h"

#include "System/Audio/SoundDataAsset.h"
#include "System/Audio/SoundManagerSubsystem.h"

void UProjectGameInstance::Init()
{
	Super::Init();

	InitializeSound();
}

void UProjectGameInstance::OnStart()
{
	Super::OnStart();

	if (bStartInitialBGM)
	{
		if (USoundManagerSubsystem* SoundManager = GetSubsystem<USoundManagerSubsystem>())
		{
			SoundManager->SetBGMState(InitialBGMState, InitialBGMVolume);
		}
	}
}

void UProjectGameInstance::Shutdown()
{
	if (USoundManagerSubsystem* SoundManager = GetSubsystem<USoundManagerSubsystem>())
	{
		SoundManager->StopBGM(false);
		SoundManager->StopAmbience(false);
	}

	Super::Shutdown();
}

void UProjectGameInstance::InitializeSound()
{
	if (bSoundInitialized)
	{
		return;
	}

	if (USoundManagerSubsystem* SoundManager = GetSubsystem<USoundManagerSubsystem>())
	{
		SoundManager->SetDebugSoundEnabled(bEnableSoundDebug);
		SoundManager->SetSoundData(SoundDataAsset);
		bSoundInitialized = true;
	}
}
