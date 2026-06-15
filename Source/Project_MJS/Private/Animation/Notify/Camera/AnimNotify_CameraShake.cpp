#include "Animation/Notify/Camera/AnimNotify_CameraShake.h"

#include "Camera/CameraDirectingComponent.h"
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

void UAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponent(MeshComp))
	{
		DirectingComponent->PlayCameraShake(ShakeClass, Scale, PlaySpace, UserPlaySpaceRot);
	}
}
