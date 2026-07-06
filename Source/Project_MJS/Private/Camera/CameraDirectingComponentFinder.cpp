#include "Camera/CameraDirectingComponentFinder.h"

#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraRigActor.h"
#include "Character/Player/CPlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

UCameraDirectingComponent* FindCameraDirectingComponentFromMesh(const USkeletalMeshComponent* MeshComp)
{
	const AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;
	const APawn* PawnOwner = Cast<APawn>(Owner);
	const UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;

	ACPlayerController* PlayerController = PawnOwner ? Cast<ACPlayerController>(PawnOwner->GetController()) : nullptr;
	if (!PlayerController && World)
	{
		PlayerController = Cast<ACPlayerController>(World->GetFirstPlayerController());
	}

	ACameraRigActor* CameraRig = PlayerController ? PlayerController->EnsureCameraRig() : nullptr;

	return CameraRig ? CameraRig->GetCameraDirectingComponent() : nullptr;
}
