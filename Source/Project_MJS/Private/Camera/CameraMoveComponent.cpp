#include "Camera/CameraMoveComponent.h"

#include "Character/Enemy/EnemyCharacter.h"
#include "GameFramework/SpringArmComponent.h"

UCameraMoveComponent::UCameraMoveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentArmLength = GetClampedDefaultArmLength();
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

	CurrentArmLength = GetClampedDefaultArmLength();
	MovementArmLengthAutoRestoreElapsedTime = 0.0f;
	bMovementArmLengthAutoRestorePending = false;

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

	if (FocusTargetActor.IsValid())
	{
		UpdateRotationToFocusTarget();
	}

	const FRotator DesiredRotation = GetCameraRotation();
	Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), DesiredRotation, DeltaTime, RotationInterpSpeed));

	if (bMovementArmLengthAutoRestorePending && bEnableMovementArmLengthAutoRestore && !bBlockMovementArmLengthAutoRestore)
	{
		UpdateMovementArmLengthAutoRestore(DeltaTime);
	}

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
	if (USpringArmComponent* SpringArmComponent = SpringArm.Get())
	{
		SpringArmComponent->TargetArmLength = GetClampedDefaultArmLength();
	}
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

void UCameraMoveComponent::SetFocusTarget(AActor* NewFocusTarget)
{
	FocusTargetActor = NewFocusTarget;
	if (FocusTargetActor.IsValid())
	{
		UpdateRotationToFocusTarget();
	}
	else if (const AActor* Owner = GetOwner())
	{
		const FRotator OwnerRotation = Owner->GetActorRotation();
		TargetYaw = OwnerRotation.Yaw;
		TargetPitch = FMath::Clamp(OwnerRotation.Pitch, MinPitch, MaxPitch);
	}
}

void UCameraMoveComponent::AddLookInput(const FVector2D& LookInput)
{
	if (LookInput.IsNearlyZero())
	{
		return;
	}

	if (bIgnoreLookInputWhileFocused && FocusTargetActor.IsValid())
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
	SetArmLength(GetClampedDefaultArmLength(), false);
}

void UCameraMoveComponent::SetArmLength(float NewArmLength, bool bApplyImmediately)
{
	CurrentArmLength = FMath::Clamp(NewArmLength, MinArmLength, MaxArmLength);
	MovementArmLengthAutoRestoreElapsedTime = 0.0f;
	bMovementArmLengthAutoRestorePending = !FMath::IsNearlyEqual(
		CurrentArmLength,
		GetClampedDefaultArmLength(),
		ArmLengthAutoRestoreTolerance);

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

void UCameraMoveComponent::UpdateMovementArmLengthAutoRestore(float DeltaTime)
{
	if (!ShouldAutoRestoreArmLengthWhileMoving())
	{
		MovementArmLengthAutoRestoreElapsedTime = 0.0f;
		if (FMath::IsNearlyEqual(CurrentArmLength, GetClampedDefaultArmLength(), ArmLengthAutoRestoreTolerance))
		{
			bMovementArmLengthAutoRestorePending = false;
		}
		return;
	}

	MovementArmLengthAutoRestoreElapsedTime += DeltaTime;
	if (MovementArmLengthAutoRestoreElapsedTime < MovementArmLengthAutoRestoreDelay)
	{
		return;
	}

	const float RestoreTargetArmLength = GetClampedDefaultArmLength();
	CurrentArmLength = FMath::FInterpTo(
		CurrentArmLength,
		RestoreTargetArmLength,
		DeltaTime,
		MovementArmLengthAutoRestoreInterpSpeed);

	if (FMath::IsNearlyEqual(CurrentArmLength, RestoreTargetArmLength, ArmLengthAutoRestoreTolerance))
	{
		CurrentArmLength = RestoreTargetArmLength;
		MovementArmLengthAutoRestoreElapsedTime = 0.0f;
		bMovementArmLengthAutoRestorePending = false;
	}
}

bool UCameraMoveComponent::ShouldAutoRestoreArmLengthWhileMoving() const
{
	if (!bMovementArmLengthAutoRestorePending)
	{
		return false;
	}

	const AActor* Target = TargetActor.Get();
	if (!Target)
	{
		return false;
	}

	const float RestoreTargetArmLength = GetClampedDefaultArmLength();
	if (FMath::IsNearlyEqual(CurrentArmLength, RestoreTargetArmLength, ArmLengthAutoRestoreTolerance))
	{
		return false;
	}

	return Target->GetVelocity().SizeSquared2D() > FMath::Square(MovementDetectSpeedThreshold);
}

float UCameraMoveComponent::GetClampedDefaultArmLength() const
{
	return FMath::Clamp(DefaultArmLength, MinArmLength, MaxArmLength);
}

void UCameraMoveComponent::UpdateRotationToFocusTarget()
{
	const AActor* Owner = GetOwner();
	if (!Owner || !FocusTargetActor.IsValid())
	{
		return;
	}

	const FVector FocusDirection = (GetFocusWorldLocation() - Owner->GetActorLocation()).GetSafeNormal();
	if (FocusDirection.IsNearlyZero())
	{
		return;
	}

	const FRotator FocusRotation = FocusDirection.Rotation();
	TargetYaw = FocusRotation.Yaw;
	TargetPitch = FMath::Clamp(FocusRotation.Pitch, MinPitch, MaxPitch);
}

FVector UCameraMoveComponent::GetFocusWorldLocation() const
{
	if (const AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(FocusTargetActor.Get()))
	{
		return EnemyCharacter->GetTargetPointLocation();
	}

	return FocusTargetActor.IsValid() ? FocusTargetActor->GetActorLocation() : FVector::ZeroVector;
}
