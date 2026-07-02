#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "CPlayerController.generated.h"

class ACameraRigActor;
class ACPlayerCharacter;
class UInputAction;
class UInputMappingContext;
class UTargetingComponent;
struct FInputActionValue;

UCLASS()
class PROJECT_MJS_API ACPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

public:
	FRotator GetCameraYawRotation() const;
	ACameraRigActor* GetCameraRig() const { return CameraRig; }
	ACameraRigActor* EnsureCameraRig();

	UFUNCTION(BlueprintPure, Category = "Cinematic|Input")
	bool IsCinematicGameplayInputLocked() const;

private:
	void InitializeCameraRig();
	ACameraRigActor* SpawnCameraRig();
	void ApplyCameraRigToCurrentPawn();
	void BindToTargetingComponent();
	void UnbindFromTargetingComponent();

	void OnMoveInput(const FInputActionValue& Value);
	void OnJumpInput();
	void OnLookInput(const FInputActionValue& Value);
	void OnDodgeInput();
	void OnAttackInput();
	void OnHardTargetInput();
	void OnRangedHardTargetTriggered();
	void OnRangedHardTargetCompleted();
	void OnRangedHardTargetCanceled();
	void OnClearHardTargetInput();

	void HandleTargetingDisplayUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void HandleTargetingDisplayCleared();
	void HandleHardTargetChanged(AActor* NewHardTarget);

	ACPlayerCharacter* GetPlayerCharacter() const;
	UTargetingComponent* GetPlayerTargetingComponent() const;
	bool IsCinematicMoveInputLocked() const;
	bool IsCinematicLookInputLocked() const;

	// ===== 입력 =====
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

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_HardTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_RangedHardTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_ClearTargeting;

	// ===== 카메라 =====
	// 플레이어 카메라를 담당할 CameraRig 클래스 (비어 있으면 기본 C++ 클래스를 사용)
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ACameraRigActor> CameraRigClass;

	UPROPERTY(Transient)
	TObjectPtr<ACameraRigActor> CameraRig;

	// 현재 델리게이트를 바인딩해 둔 타겟팅 컴포넌트
	UPROPERTY(Transient)
	TWeakObjectPtr<UTargetingComponent> BoundTargetingComponent;
};
