#include "Camera/CameraRigActor.h"

#include "Camera/CameraMoveComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraDirectingComponent.h"
#include "GameFramework/SpringArmComponent.h"

ACameraRigActor::ACameraRigActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	RootComponent = SpringArm;
	
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 0.0f);
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bDoCollisionTest = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraLagSpeed = 14.0f;
	SpringArm->CameraRotationLagSpeed = 18.0f;
	SpringArm->CameraLagMaxDistance = 120.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 78.0f;

	CameraMoveComp = CreateDefaultSubobject<UCameraMoveComponent>(TEXT("CameraMove"));
	CameraMoveComp->Initialize(SpringArm);
	
	CameraDirectingComp = CreateDefaultSubobject<UCameraDirectingComponent>(TEXT("CameraDirecting"));
	CameraDirectingComp->Initialize(Camera);
}

void ACameraRigActor::SetCameraTarget(AActor* NewTarget)
{
	if (CameraMoveComp)
	{
		CameraMoveComp->SetCameraTarget(NewTarget);
	}
}

AActor* ACameraRigActor::GetCurrentTarget() const
{
	return CameraMoveComp ? CameraMoveComp->GetCurrentTarget() : nullptr;
}

void ACameraRigActor::SetFocusTarget(AActor* NewFocusTarget)
{
	if (CameraMoveComp)
	{
		CameraMoveComp->SetFocusTarget(NewFocusTarget);
	}
}

AActor* ACameraRigActor::GetFocusTarget() const
{
	return CameraMoveComp ? CameraMoveComp->GetFocusTarget() : nullptr;
}

void ACameraRigActor::AddLookInput(const FVector2D& LookInput)
{
	if (CameraMoveComp)
	{
		CameraMoveComp->AddLookInput(LookInput);
	}
}

void ACameraRigActor::AdjustZoom(float Delta)
{
	if (CameraMoveComp)
	{
		CameraMoveComp->AdjustZoom(Delta);
	}
}

void ACameraRigActor::ResetZoom()
{
	if (CameraMoveComp)
	{
		CameraMoveComp->ResetZoom();
	}
}

void ACameraRigActor::SetArmLength(float NewArmLength, bool bApplyImmediately)
{
	if (CameraMoveComp)
	{
		CameraMoveComp->SetArmLength(NewArmLength, bApplyImmediately);
	}
}

FRotator ACameraRigActor::GetCameraYawRotation() const
{
	return CameraMoveComp ? CameraMoveComp->GetCameraYawRotation() : FRotator::ZeroRotator;
}
