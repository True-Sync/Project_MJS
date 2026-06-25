#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraMoveComponent.generated.h"

class USpringArmComponent;

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UCameraMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraMoveComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Initialize(USpringArmComponent* InSpringArm);
	void SetCameraTarget(AActor* NewTarget);
	AActor* GetCurrentTarget() const { return TargetActor.Get(); }
	void SetFocusTarget(AActor* NewFocusTarget);
	AActor* GetFocusTarget() const { return FocusTargetActor.Get(); }

	void AddLookInput(const FVector2D& LookInput);
	void AdjustZoom(float Delta);
	void ResetZoom();
	void SetArmLength(float NewArmLength, bool bApplyImmediately = true);

	FRotator GetCameraRotation() const;
	FRotator GetCameraYawRotation() const;

private:
	void UpdateRotationToFocusTarget();
	FVector GetFocusWorldLocation() const;

	UPROPERTY(VisibleAnywhere, Category = "Camera|Follow")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, Category = "Camera|Focus")
	TWeakObjectPtr<AActor> FocusTargetActor;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TWeakObjectPtr<USpringArmComponent> SpringArm;

	// ===== 추적 =====
	UPROPERTY(EditAnywhere, Category = "Camera|Follow")
	FVector TargetOffset = FVector(0.0f, 0.0f, 85.0f);

	UPROPERTY(EditAnywhere, Category = "Camera|Follow", meta = (ClampMin = "1.0"))
	float LocationInterpSpeed = 18.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Follow", meta = (ClampMin = "1.0"))
	float RotationInterpSpeed = 20.0f;

	// ===== 회전 =====
	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "0.01"))
	float YawSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "0.01"))
	float PitchSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinPitch = -60.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxPitch = 35.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Focus")
	bool bIgnoreLookInputWhileFocused = true;

	// ===== 줌 =====
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float ArmLengthInterpSpeed = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float ZoomStep = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float MinArmLength = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float DefaultArmLength = 480.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float MaxArmLength = 650.0f;

	float TargetYaw = 0.0f;
	float TargetPitch = 0.0f;
	float CurrentArmLength = 480.0f;
};
