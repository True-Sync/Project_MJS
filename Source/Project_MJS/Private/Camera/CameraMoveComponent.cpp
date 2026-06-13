#include "Camera/CameraMoveComponent.h"

#include "GameFramework/SpringArmComponent.h"

UCameraMoveComponent::UCameraMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AActor* Owner = GetOwner())
	{
		const FRotator OwnerRotation = Owner->GetActorRotation();
		TargetYaw = OwnerRotation.Yaw;
		TargetPitch = FMath::Clamp(OwnerRotation.Pitch, MinPitch, MaxPitch);
	}

	CurrentArmLength = FMath::Clamp(DefaultArmLength, MinArmLength, MaxArmLength);
	if (USpringArmComponent* SpringArmComponent = SpringArm.Get())
	{
		SpringArmComponent->TargetArmLength = CurrentArmLength;
	}
}

void UCameraMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (AActor* Target = TargetActor.Get())
	{
		const FVector DesiredLocation = Target->GetActorLocation() + TargetOffset;
		Owner->SetActorLocation(FMath::VInterpTo(Owner->GetActorLocation(), DesiredLocation, DeltaTime, LocationInterpSpeed));
	}

	const FRotator DesiredRotation = GetCameraRotation();
	Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), DesiredRotation, DeltaTime, RotationInterpSpeed));

	if (USpringArmComponent* SpringArmComponent = SpringArm.Get())
	{
		SpringArmComponent->TargetArmLength = FMath::FInterpTo(
			SpringArmComponent->TargetArmLength,
			CurrentArmLength,
			DeltaTime,
			ArmLengthInterpSpeed);
	}
}

void UCameraMoveComponent::Initialize(USpringArmComponent* InSpringArm)
{
	SpringArm = InSpringArm;
}

void UCameraMoveComponent::SetCameraTarget(AActor* NewTarget)
{
	TargetActor = NewTarget;
	if (NewTarget)
	{
		if (AActor* Owner = GetOwner())
		{
			Owner->SetActorLocation(NewTarget->GetActorLocation() + TargetOffset);
		}
	}
}

void UCameraMoveComponent::AddLookInput(const FVector2D& LookInput)
{
	if (LookInput.IsNearlyZero())
	{
		return;
	}

	TargetYaw += LookInput.X * YawSensitivity;
	TargetPitch = FMath::Clamp(TargetPitch - (LookInput.Y * PitchSensitivity), MinPitch, MaxPitch);
}

void UCameraMoveComponent::AdjustZoom(float Delta)
{
	SetArmLength(CurrentArmLength + (Delta * ZoomStep), false);
}

void UCameraMoveComponent::ResetZoom()
{
	SetArmLength(DefaultArmLength, false);
}

void UCameraMoveComponent::SetArmLength(float NewArmLength, bool bApplyImmediately)
{
	CurrentArmLength = FMath::Clamp(NewArmLength, MinArmLength, MaxArmLength);
	if (bApplyImmediately)
	{
		if (USpringArmComponent* SpringArmComponent = SpringArm.Get())
		{
			SpringArmComponent->TargetArmLength = CurrentArmLength;
		}
	}
}

FRotator UCameraMoveComponent::GetCameraRotation() const
{
	return FRotator(TargetPitch, TargetYaw, 0.0f);
}

FRotator UCameraMoveComponent::GetCameraYawRotation() const
{
	return FRotator(0.0f, TargetYaw, 0.0f);
}
