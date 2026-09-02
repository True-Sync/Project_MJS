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
	void UpdateMovementArmLengthAutoRestore(float DeltaTime);
	bool ShouldAutoRestoreArmLengthWhileMoving() const;
	float GetClampedDefaultArmLength() const;
	FVector GetFocusWorldLocation() const;

	UPROPERTY(VisibleAnywhere, Category = "Camera|Follow")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, Category = "Camera|Focus")
	TWeakObjectPtr<AActor> FocusTargetActor;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TWeakObjectPtr<USpringArmComponent> SpringArm;

	// ===== 추적 =====
	// 타겟 위치에서 카메라 리그가 유지할 오프셋
	UPROPERTY(EditAnywhere, Category = "Camera|Follow")
	FVector TargetOffset = FVector(0.0f, 0.0f, 85.0f);

	// 카메라 리그가 타겟 위치를 따라가는 보간 속도
	UPROPERTY(EditAnywhere, Category = "Camera|Follow", meta = (ClampMin = "1.0"))
	float LocationInterpSpeed = 18.0f;

	// 카메라 리그가 목표 회전으로 따라가는 보간 속도
	UPROPERTY(EditAnywhere, Category = "Camera|Follow", meta = (ClampMin = "1.0"))
	float RotationInterpSpeed = 20.0f;

	// ===== 회전 =====
	// 좌우 카메라 입력 민감도
	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "0.01"))
	float YawSensitivity = 1.0f;

	// 상하 카메라 입력 민감도
	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "0.01"))
	float PitchSensitivity = 1.0f;

	// 카메라가 아래로 내려갈 수 있는 최소 피치 각도
	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MinPitch = -60.0f;

	// 카메라가 위로 올라갈 수 있는 최대 피치 각도
	UPROPERTY(EditAnywhere, Category = "Camera|Rotation", meta = (ClampMin = "-89.0", ClampMax = "89.0"))
	float MaxPitch = 35.0f;

	// 포커스 대상이 있을 때 수동 시점 입력을 무시할지 여부
	UPROPERTY(EditAnywhere, Category = "Camera|Focus")
	bool bIgnoreLookInputWhileFocused = true;

	// ===== 줌 =====
	// 실제 스프링암 길이가 CurrentArmLength를 따라가는 보간 속도
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float ArmLengthInterpSpeed = 8.0f;

	// 줌 입력 한 칸당 변경되는 카메라 거리
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "1.0"))
	float ZoomStep = 80.0f;

	// 카메라가 가장 가까워질 수 있는 최소 거리
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float MinArmLength = 250.0f;

	// 자동 보정과 ResetZoom이 돌아갈 기본 카메라 거리
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float DefaultArmLength = 480.0f;

	// 카메라가 가장 멀어질 수 있는 최대 거리
	UPROPERTY(EditAnywhere, Category = "Camera|Zoom", meta = (ClampMin = "100.0"))
	float MaxArmLength = 650.0f;

	// ===== 자동 거리 보정 =====
	// 자동 거리 보정이 목표 거리에 도달했다고 판단하는 허용 오차
	static constexpr float ArmLengthAutoRestoreTolerance = 0.5f;
	
	// 이동 중 카메라 거리를 DefaultArmLength로 자동 보정할지 여부
	UPROPERTY(EditAnywhere, Category = "Camera|Auto Restore")
	bool bEnableMovementArmLengthAutoRestore = true;

	// 자동 거리 보정을 일시적으로 막는 옵션(추후 전투 상태와 연결할 수 있음)
	UPROPERTY(EditAnywhere, Category = "Camera|Auto Restore")
	bool bBlockMovementArmLengthAutoRestore = false;

	// 이동이 시작된 뒤 자동 거리 보정이 시작되기까지의 대기 시간
	UPROPERTY(EditAnywhere, Category = "Camera|Auto Restore", meta = (ClampMin = "0.0"))
	float MovementArmLengthAutoRestoreDelay = 2.0f;

	// CurrentArmLength가 DefaultArmLength로 돌아가는 자동 보정 속도
	UPROPERTY(EditAnywhere, Category = "Camera|Auto Restore", meta = (ClampMin = "0.01"))
	float MovementArmLengthAutoRestoreInterpSpeed = 0.35f;

	// 이 속도 이상으로 타겟이 움직일 때 이동 중으로 판단
	UPROPERTY(EditAnywhere, Category = "Camera|Auto Restore", meta = (ClampMin = "0.0"))
	float MovementDetectSpeedThreshold = 10.0f;

	// ===== 런타임 상태 =====
	float TargetYaw = 0.0f;
	float TargetPitch = 0.0f;
	float CurrentArmLength = 0.0f;
	// 이동 중 자동 거리 보정을 시작하기 전까지 누적한 시간
	float MovementArmLengthAutoRestoreElapsedTime = 0.0f;
	// 줌/거리 변경 이후 자동 거리 보정이 필요한 상태인지 여부
	bool bMovementArmLengthAutoRestorePending = false;
};
