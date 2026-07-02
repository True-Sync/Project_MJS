#include "Animation/Notify/Camera/AnimNotifyState_CameraFOV.h"

#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraDirectingComponentFinder.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_CameraFOV::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponentFromMesh(MeshComp))
	{
		DirectingComponent->PlayFOV(TargetFOV, BlendInTime, 0.0f, BlendOutTime, false, bUseExplicitStartFOV, ExplicitStartFOV, BlendType);
	}
}

void UAnimNotifyState_CameraFOV::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponentFromMesh(MeshComp))
	{
		DirectingComponent->EndFOV(BlendOutTime, BlendType);
	}
}
