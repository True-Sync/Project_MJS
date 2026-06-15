#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPlayerController.generated.h"

class ACameraRigActor;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS()
class PROJECT_MJS_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
	
public:
	FRotator GetCameraYawRotation() const;

	UFUNCTION(BlueprintPure, Category = "Cinematic|Input")
	bool IsCinematicGameplayInputLocked() const;

private:
	// 초기 카메라 세팅 함수
	void InitializeCameraRig();
	
	void OnMoveInput(const FInputActionValue& Value);
	void OnJumpInput();
	void OnLookInput(const FInputActionValue& Value);
	void OnDodgeInput();
	void OnAttackInput();
	bool IsCinematicMoveInputLocked() const;
	bool IsCinematicLookInputLocked() const;

	// ===== Input =====
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Move;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Jump;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Look;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dodge;

	// ===== 카메라 =====
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ACameraRigActor> CameraRigClass;

	UPROPERTY(Transient)
	TObjectPtr<ACameraRigActor> CameraRig;
};
