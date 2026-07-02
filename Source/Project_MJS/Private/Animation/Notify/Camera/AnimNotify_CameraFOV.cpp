#include "Animation/Notify/Camera/AnimNotify_CameraFOV.h"

#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraDirectingComponentFinder.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_CameraFOV::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponentFromMesh(MeshComp))
	{
		DirectingComponent->PlayFOV(TargetFOV, BlendInTime, HoldTime, BlendOutTime, bRestoreAfterHold, bUseExplicitStartFOV, ExplicitStartFOV, BlendType);
	}
}
