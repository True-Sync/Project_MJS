#include "Animation/Notify/Camera/AnimNotify_CameraFOV.h"

#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraRigActor.h"
#include "Character/Player/CPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"

void UAnimNotify_CameraFOV::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponent(MeshComp))
	{
		DirectingComponent->PlayFOV(TargetFOV, BlendInTime, HoldTime, BlendOutTime, bRestoreAfterHold, bUseExplicitStartFOV, ExplicitStartFOV, BlendType);
	}
}

UCameraDirectingComponent* UAnimNotify_CameraFOV::FindCameraDirectingComponent(const USkeletalMeshComponent* MeshComp)
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
