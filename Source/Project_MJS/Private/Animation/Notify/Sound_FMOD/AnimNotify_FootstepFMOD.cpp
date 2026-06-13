#include "Animation/Notify/Sound_FMOD/AnimNotify_FootstepFMOD.h"

#include "System/Audio/SoundDataAsset.h"
#include "System/Audio/SoundManagerSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FMODEvent.h"

void UAnimNotify_FootstepFMOD::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
	USoundManagerSubsystem* SoundManager = GameInstance ? GameInstance->GetSubsystem<USoundManagerSubsystem>() : nullptr;
	if (!SoundManager)
	{
		return;
	}

	UFMODEvent* FootstepEvent = EventOverride;
	if (!FootstepEvent)
	{
		if (USoundDataAsset* SoundData = SoundManager->GetSoundData())
		{
			FootstepEvent = SoundData->Player.Footstep;
		}
	}

	if (!FootstepEvent)
	{
		return;
	}

	const FVector Location = SocketName.IsNone()
		? MeshComp->GetComponentLocation()
		: MeshComp->GetSocketLocation(SocketName);

	SoundManager->PlaySFXAtLocation(FootstepEvent, Location, Volume, Cooldown);
}
