#include "Camera/CameraDirectingComponent.h"

#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

UCameraDirectingComponent::UCameraDirectingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


void UCameraDirectingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UCameraComponent* Camera = CameraComp.Get())
	{
		DefaultFOV = Camera->FieldOfView;
		FOVStart = DefaultFOV;
		FOVTarget = DefaultFOV;
		FOVRestoreTarget = DefaultFOV;
	}
}

void UCameraDirectingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FOVPhase == EFOVPhase::None)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (!CameraComp.IsValid())
	{
		FOVPhase = EFOVPhase::None;
		SetComponentTickEnabled(false);
		return;
	}

	if (FOVPhase == EFOVPhase::Hold)
	{
		FOVElapsed += DeltaTime;
		if (FOVHoldDuration <= 0.0f || FOVElapsed >= FOVHoldDuration)
		{
			if (bRestoreFOVAfterHold)
			{
				SetFOVPhase(EFOVPhase::BlendOut, FOVBlendOutDuration, CameraComp->FieldOfView, FOVRestoreTarget);
			}
			else
			{
				FOVPhase = EFOVPhase::None;
				SetComponentTickEnabled(false);
			}
		}

		return;
	}

	FOVElapsed += DeltaTime;
	const float Alpha = FOVPhaseDuration > 0.0f ? FMath::Clamp(FOVElapsed / FOVPhaseDuration, 0.0f, 1.0f) : 1.0f;
	ApplyFOVAlpha(EvaluateBlend(Alpha));

	if (Alpha < 1.0f)
	{
		return;
	}

	if (FOVPhase == EFOVPhase::BlendIn)
	{
		if (FOVHoldDuration > 0.0f || bRestoreFOVAfterHold)
		{
			SetFOVPhase(EFOVPhase::Hold, FOVHoldDuration, FOVTarget, FOVTarget);
		}
		else
		{
			FOVPhase = EFOVPhase::None;
			SetComponentTickEnabled(false);
		}
	}
	else if (FOVPhase == EFOVPhase::BlendOut)
	{
		FOVPhase = EFOVPhase::None;
		SetComponentTickEnabled(false);
	}
}

void UCameraDirectingComponent::Initialize(UCameraComponent* InCameraComp)
{
	CameraComp = InCameraComp;
}

void UCameraDirectingComponent::PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale, ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot)
{
	if (!ShakeClass)
	{
		return;
	}

	const UWorld* World = GetWorld();
	APlayerController* PlayerController = World ? World->GetFirstPlayerController() : nullptr;
	APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (CameraManager)
	{
		CameraManager->StartCameraShake(ShakeClass, Scale, PlaySpace, UserPlaySpaceRot);
	}
}

void UCameraDirectingComponent::PlayFOV(float TargetFOV, float BlendInTime, float HoldTime, float BlendOutTime, bool bRestoreAfterHold, bool bUseExplicitStartFOV, float ExplicitStartFOV, ECameraDirectingFOVBlendType BlendType)
{
	UCameraComponent* Camera = CameraComp.Get();
	if (!Camera)
	{
		return;
	}

	ActiveBlendType = BlendType;
	FOVRestoreTarget = DefaultFOV;
	FOVHoldDuration = FMath::Max(0.0f, HoldTime);
	FOVBlendOutDuration = FMath::Max(0.0f, BlendOutTime);
	bRestoreFOVAfterHold = bRestoreAfterHold;

	const float StartFOV = bUseExplicitStartFOV ? ExplicitStartFOV : Camera->FieldOfView;
	if (bUseExplicitStartFOV)
	{
		Camera->SetFieldOfView(StartFOV);
	}

	SetFOVPhase(EFOVPhase::BlendIn, FMath::Max(0.0f, BlendInTime), StartFOV, TargetFOV);
}

void UCameraDirectingComponent::EndFOV(float BlendOutTime, ECameraDirectingFOVBlendType BlendType)
{
	UCameraComponent* Camera = CameraComp.Get();
	if (!Camera)
	{
		return;
	}

	ActiveBlendType = BlendType;
	SetFOVPhase(EFOVPhase::BlendOut, FMath::Max(0.0f, BlendOutTime), Camera->FieldOfView, FOVRestoreTarget);
}

void UCameraDirectingComponent::SetFOVImmediately(float NewFOV)
{
	if (UCameraComponent* Camera = CameraComp.Get())
	{
		FOVPhase = EFOVPhase::None;
		SetComponentTickEnabled(false);
		Camera->SetFieldOfView(NewFOV);
	}
}

void UCameraDirectingComponent::ResetFOV(float BlendTime, ECameraDirectingFOVBlendType BlendType)
{
	UCameraComponent* Camera = CameraComp.Get();
	if (!Camera)
	{
		return;
	}

	ActiveBlendType = BlendType;
	SetFOVPhase(EFOVPhase::BlendOut, FMath::Max(0.0f, BlendTime), Camera->FieldOfView, DefaultFOV);
}

void UCameraDirectingComponent::SetFOVPhase(EFOVPhase NewPhase, float Duration, float NewStartFOV, float NewTargetFOV)
{
	FOVPhase = NewPhase;
	FOVElapsed = 0.0f;
	FOVPhaseDuration = Duration;
	FOVStart = NewStartFOV;
	FOVTarget = NewTargetFOV;
	SetComponentTickEnabled(NewPhase != EFOVPhase::None);

	if (Duration <= 0.0f && NewPhase != EFOVPhase::Hold)
	{
		ApplyFOVAlpha(1.0f);
	}
}

void UCameraDirectingComponent::ApplyFOVAlpha(float Alpha) const
{
	if (UCameraComponent* Camera = CameraComp.Get())
	{
		Camera->SetFieldOfView(FMath::Lerp(FOVStart, FOVTarget, Alpha));
	}
}

float UCameraDirectingComponent::EvaluateBlend(float Alpha) const
{
	switch (ActiveBlendType)
	{
	case ECameraDirectingFOVBlendType::EaseIn:
		return FMath::InterpEaseIn(0.0f, 1.0f, Alpha, 2.0f);
	case ECameraDirectingFOVBlendType::EaseOut:
		return FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
	case ECameraDirectingFOVBlendType::EaseInOut:
		return FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
	case ECameraDirectingFOVBlendType::Linear:
	default:
		return Alpha;
	}
}

