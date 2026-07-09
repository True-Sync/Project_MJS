#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraRigActor.generated.h"

class UCameraDirectingComponent;
class UCameraComponent;
class UCameraMoveComponent;
class USpringArmComponent;

UCLASS()
class PROJECT_MJS_API ACameraRigActor : public AActor
{
	GENERATED_BODY()

public:
	ACameraRigActor();

	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// 카메라 위치, 회전, 줌 거리 같은 이동 관련 처리를 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraMoveComponent> CameraMoveComp;

	// 카메라 쉐이크, FOV 연출 같은 연출성 처리를 담당
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraDirectingComponent> CameraDirectingComp;
	
	void SetCameraTarget(AActor* NewTarget);
	AActor* GetCurrentTarget() const;
	void SetFocusTarget(AActor* NewFocusTarget);
	AActor* GetFocusTarget() const;

	void AddLookInput(const FVector2D& LookInput);
	void AdjustZoom(float Delta);
	void ResetZoom();
	void SetArmLength(float NewArmLength, bool bApplyImmediately = true);
	FRotator GetCameraYawRotation() const;
	UCameraDirectingComponent* GetCameraDirectingComponent() const { return CameraDirectingComp; }
};
