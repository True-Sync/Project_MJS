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

	virtual void BeginPlay() override;

	// ===== Component =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraMoveComponent> CameraMoveComp;

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
