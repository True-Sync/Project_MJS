#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "CPlayerController.generated.h"

class ACameraRigActor;
class ACommandBoxActor;
class ACPlayerCharacter;
class UInputAction;
class UInputMappingContext;
class UInteractionComponent;
class UHousingPlacementComponent;
class USkillDataAsset;
class UTargetingComponent;
class UUserWidget;
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
	ACPlayerController();

	FRotator GetCameraYawRotation() const;
	ACameraRigActor* GetCameraRig() const { return CameraRig; }
	ACameraRigActor* EnsureCameraRig();

	UFUNCTION(BlueprintPure, Category = "Cinematic|Input")
	bool IsCinematicGameplayInputLocked() const;

	void RequestTogglePause();

	void RequestResumeGame();
	void OpenCommandBoxMenu(ACommandBoxActor* CommandBox);
	void CloseCommandBoxMenu();
	void RequestCommandBoxHousing();
	void RequestCommandBoxCostume();
	void RequestCommandBoxStageTravel();
	bool IsCommandBoxMenuOpen() const { return bCommandBoxMenuOpen; }
	UHousingPlacementComponent* GetHousingPlacementComponent() const { return HousingPlacementComponent; }

private:
	ACameraRigActor* SpawnCameraRig();
	void ApplyCameraRigToCurrentPawn();
	void BindToTargetingComponent();
	void UnbindFromTargetingComponent();

	void OnMoveInput(const FInputActionValue& Value);
	void OnJumpInput();
	void OnLookInput(const FInputActionValue& Value);
	void OnCameraZoomInput(const FInputActionValue& Value);
	void OnAttackInput();
	void OnSkill1Input();
	void OnSkill2Input();
	void OnDodgeInput();
	void OnHardTargetInput();
	void OnRangedHardTargetTriggered();
	void OnRangedHardTargetCompleted();
	void OnRangedHardTargetCanceled();
	void OnClearHardTargetInput();
	void OnPauseInput();
	void OnInteractInput();

	void HandleTargetingDisplayUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void HandleTargetingDisplayCleared();
	void HandleHardTargetChanged(AActor* NewHardTarget);

	ACPlayerCharacter* GetPlayerCharacter() const;
	UTargetingComponent* GetPlayerTargetingComponent() const;
	UInteractionComponent* GetPlayerInteractionComponent() const;
	bool IsCinematicMoveInputLocked() const;
	bool IsCinematicLookInputLocked() const;
	bool IsModalGameplayInputLocked() const;
	void SetGameplayPaused(bool bShouldPause);
	void AddDefaultInputMappingContext();
	void RemoveDefaultInputMappingContext();

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
	TObjectPtr<UInputAction> IA_CameraZoom;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Attack;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_Dodge;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	TObjectPtr<UInputAction> IA_Skill1;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Skill")
	TObjectPtr<UInputAction> IA_Skill2;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_HardTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_RangedHardTarget;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Targeting")
	TObjectPtr<UInputAction> IA_ClearTargeting;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input|Pause")
	TObjectPtr<UInputAction> IA_Pause;

	UPROPERTY(EditDefaultsOnly, Category = "Input|Interaction")
	TObjectPtr<UInputAction> IA_Interact;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TObjectPtr<USkillDataAsset> Skill1Data;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TObjectPtr<USkillDataAsset> Skill2Data;

	// ===== 카메라 =====
	// 플레이어 카메라를 담당할 CameraRig 클래스. 비어 있으면 기본 C++ 클래스를 사용한다.
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ACameraRigActor> CameraRigClass;

	UPROPERTY(Transient)
	TObjectPtr<ACameraRigActor> CameraRig;

	// 현재 델리게이트를 바인딩해 둔 타겟팅 컴포넌트.
	UPROPERTY(Transient)
	TWeakObjectPtr<UTargetingComponent> BoundTargetingComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ACommandBoxActor> ActiveCommandBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Housing", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHousingPlacementComponent> HousingPlacementComponent;

	bool bCommandBoxMenuOpen = false;

public:
	UFUNCTION(BlueprintCallable, Category = "DevConsole")
	void ToggleDevConsole();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Input|Debug")
	TObjectPtr<UInputAction> IA_ToggleDevConsole;

	// 개발 콘솔 UMG 위젯 클래스 (BP에서 할당)
	UPROPERTY(EditDefaultsOnly, Category = "Debug|UI", meta = (AllowedClasses = "/Script/UMG.UserWidget"))
	TSubclassOf<UUserWidget> DevConsoleWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> DevConsoleWidget;

	bool bIsDevConsoleOpen = false;
};
