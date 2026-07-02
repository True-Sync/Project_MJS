#include "Animation/Notify/Camera/AnimNotify_CameraShake.h"

#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraDirectingComponentFinder.h"
#include "Components/SkeletalMeshComponent.h"


void UAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UCameraDirectingComponent* DirectingComponent = FindCameraDirectingComponentFromMesh(MeshComp))
	{
		DirectingComponent->PlayCameraShake(ShakeClass, Scale, PlaySpace, UserPlaySpaceRot);
	}
}
