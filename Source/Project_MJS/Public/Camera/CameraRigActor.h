#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraRigActor.generated.h"

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraMoveComponent> CameraMoveComp;

	void SetCameraTarget(AActor* NewTarget);
	AActor* GetCurrentTarget() const;

	void AddLookInput(const FVector2D& LookInput);
	void AdjustZoom(float Delta);
	void ResetZoom();
	void SetArmLength(float NewArmLength, bool bApplyImmediately = true);
	FRotator GetCameraYawRotation() const;
};
