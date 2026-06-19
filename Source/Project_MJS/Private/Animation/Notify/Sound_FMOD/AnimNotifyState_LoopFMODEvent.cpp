#include "Animation/Notify/Sound_FMOD/AnimNotifyState_LoopFMODEvent.h"

#include "System/Audio/SoundManagerSubsystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "FMODEvent.h"

void UAnimNotifyState_LoopFMODEvent::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

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

	SoundManager->PlaySFXAttached(GetEffectiveHandle(MeshComp), Event, MeshComp, AttachPointName, Volume);
}

void UAnimNotifyState_LoopFMODEvent::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

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

	SoundManager->StopAttachedSFX(GetEffectiveHandle(MeshComp));
}

FName UAnimNotifyState_LoopFMODEvent::GetEffectiveHandle(USkeletalMeshComponent* MeshComp) const
{
	if (!Handle.IsNone())
	{
		return Handle;
	}

	return FName(*FString::Printf(TEXT("%s_%s"), *GetName(), *GetNameSafe(MeshComp ? MeshComp->GetOwner() : nullptr)));
}
