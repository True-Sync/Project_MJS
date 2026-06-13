#include "Animation/Notify/Sound_FMOD/AnimNotify_PlayFMODEvent.h"

#include "System/Audio/SoundManagerSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FMODEvent.h"

void UAnimNotify_PlayFMODEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !Event)
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

	const FVector Location = SocketName.IsNone()
		? MeshComp->GetComponentLocation()
		: MeshComp->GetSocketLocation(SocketName);

	SoundManager->PlaySFXAtLocation(Event, Location, Volume, Cooldown);
}
