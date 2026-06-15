#include "Animation/Notify/Camera/AnimNotifyState_CameraFOV.h"

#include "Camera/CameraRigActor.h"
#include "Character/Player/CPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"

namespace
{
UCameraDirectingComponent* FindCameraDirectingComponent(const USkeletalMeshComponent* MeshComp)
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	const APawn* PawnOwner = Cast<APawn>(Owner);
	const UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;

	ACPlayerController* PlayerController = PawnOwner ? Cast<ACPlayerController>(PawnOwner->GetController()) : nullptr;
	if (!PlayerController && World)
	{
		PlayerController = Cast<ACPlayerController>(World->GetFirstPlayerController());
	}

	ACameraRigActor* CameraRig = PlayerController ? PlayerController->GetCameraRig() : nullptr;
	if (!CameraRig && World)
	{
		for (TActorIterator<ACameraRigActor> It(World); It; ++It)
		{
			CameraRig = *It;
			break;
		}
	}

	return CameraRig ? CameraRig->GetCameraDirectingComponent() : nullptr;
}
}

void UAnimNotifyState_CameraFOV::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponent(MeshComp))
	{
		DirectingComponent->PlayFOV(TargetFOV, BlendInTime, 0.0f, BlendOutTime, false, bUseExplicitStartFOV, ExplicitStartFOV, BlendType);
	}
}

void UAnimNotifyState_CameraFOV::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponent(MeshComp))
	{
		DirectingComponent->EndFOV(BlendOutTime, BlendType);
	}
}
