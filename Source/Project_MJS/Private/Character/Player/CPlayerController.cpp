// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CPlayerController.h"

#include "Camera/CameraRigActor.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/Player/CPlayerHUD.h"
#include "Character/Player/Component/PlayerMovementComponent.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "CommandBox/CommandBoxActor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Interaction/Interactable.h"
#include "Interaction/InteractionComponent.h"
#include "Housing/HousingPlacementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PauseMenuWidget.h"
#include "UI/CommandBoxMenuWidget.h"

#if !UE_BUILD_SHIPPING
#include "System/Debug/DevConsoleSubsystem.h"
#include "Blueprint/UserWidget.h"
#endif // !UE_BUILD_SHIPPING

ACPlayerController::ACPlayerController()
{
	HousingPlacementComponent = CreateDefaultSubobject<UHousingPlacementComponent>(TEXT("HousingPlacementComponent"));
}

void ACPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetInputMode(FInputModeGameOnly());
	
	AddDefaultInputMappingContext();

	EnsureCameraRig();
	BindToTargetingComponent();
}

void ACPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HousingPlacementComponent)
	{
		HousingPlacementComponent->ExitHousing();
	}

	if (bCommandBoxMenuOpen)
	{
		CloseCommandBoxMenu();
	}

	ActiveCommandBox.Reset();
	UnbindFromTargetingComponent();
	RemoveDefaultInputMappingContext();
	Super::EndPlay(EndPlayReason);
}

void ACPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	EnsureCameraRig();
	BindToTargetingComponent();
}

void ACPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent failed: InputComponent is not UEnhancedInputComponent."));
		return;
	}

	if (HousingPlacementComponent)
	{
		HousingPlacementComponent->InitializeInput(EnhancedInputComponent);
	}

	if (IA_Move)
	{
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ACPlayerController::OnMoveInput);
		EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &ACPlayerController::OnMoveInput);
	}
	
	if (IA_Jump)
	{
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACPlayerController::OnJumpInput);
	}
	
	if (IA_Look)
	{
		EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ACPlayerController::OnLookInput);
	}
	
	if (IA_CameraZoom)
	{
		EnhancedInputComponent->BindAction(IA_CameraZoom, ETriggerEvent::Triggered, this, &ACPlayerController::OnCameraZoomInput);
	}
	
	if (IA_Attack)
	{
		EnhancedInputComponent->BindAction(IA_Attack, ETriggerEvent::Started, this, &ACPlayerController::OnAttackInput);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent: IA_Attack is not assigned."));
	}
	
	if (IA_Dodge)
	{
		EnhancedInputComponent->BindAction(IA_Dodge, ETriggerEvent::Started, this, &ACPlayerController::OnDodgeInput);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent: IA_Dodge is not assigned."));
	}


	if (IA_Skill1)
	{
		EnhancedInputComponent->BindAction(IA_Skill1, ETriggerEvent::Started, this, &ACPlayerController::OnSkill1Input);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent: IA_SKill1 is not assigned."));
	}

	if (IA_Skill2)
	{
		EnhancedInputComponent->BindAction(IA_Skill2, ETriggerEvent::Started, this, &ACPlayerController::OnSkill2Input);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SetupInputComponent: IA_SKill2 is not assigned."));
	}
	
	if (IA_HardTarget)
	{
		EnhancedInputComponent->BindAction(IA_HardTarget, ETriggerEvent::Started, this, &ACPlayerController::OnHardTargetInput);
	}

	if (IA_RangedHardTarget)
	{
		EnhancedInputComponent->BindAction(IA_RangedHardTarget, ETriggerEvent::Triggered, this, &ACPlayerController::OnRangedHardTargetTriggered);
		EnhancedInputComponent->BindAction(IA_RangedHardTarget, ETriggerEvent::Completed, this, &ACPlayerController::OnRangedHardTargetCompleted);
		EnhancedInputComponent->BindAction(IA_RangedHardTarget, ETriggerEvent::Canceled, this, &ACPlayerController::OnRangedHardTargetCanceled);
	}

	if (IA_ClearTargeting)
	{
		EnhancedInputComponent->BindAction(IA_ClearTargeting, ETriggerEvent::Started, this, &ACPlayerController::OnClearHardTargetInput);
	}
	
	if (IA_Pause)
	{
		EnhancedInputComponent->BindAction(IA_Pause, ETriggerEvent::Started, this, &ACPlayerController::OnPauseInput);
	}

	if (IA_Interact)
	{
		EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &ACPlayerController::OnInteractInput);
	}

#if !UE_BUILD_SHIPPING
	if (IA_ToggleDevConsole)
	{
		EnhancedInputComponent->BindAction(IA_ToggleDevConsole, ETriggerEvent::Started, this, &ACPlayerController::ToggleDevConsole);
	}
#endif // !UE_BUILD_SHIPPING
}	

FRotator ACPlayerController::GetCameraYawRotation() const
{
	return CameraRig ? CameraRig->GetCameraYawRotation() : FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
}

ACameraRigActor* ACPlayerController::EnsureCameraRig()
{
	if (!IsValid(CameraRig))
	{
		if (CameraRig)
		{
			UE_LOG(LogTemp, Warning, TEXT("CameraRig was invalid. Respawning CameraRig."));
		}
		CameraRig = SpawnCameraRig();
	}

	ApplyCameraRigToCurrentPawn();
	return CameraRig;
}

ACameraRigActor* ACPlayerController::SpawnCameraRig()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UClass* SpawnClass = CameraRigClass ? CameraRigClass.Get() : ACameraRigActor::StaticClass();
	const FVector RigSpawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetPawn();

	return World->SpawnActor<ACameraRigActor>(SpawnClass, RigSpawnLocation, FRotator::ZeroRotator, SpawnParams);
}

void ACPlayerController::ApplyCameraRigToCurrentPawn()
{
	if (!CameraRig)
	{
		return;
	}

	CameraRig->SetCameraTarget(GetPawn());
	if (GetViewTarget() != CameraRig)
	{
		SetViewTarget(CameraRig);
	}
}

void ACPlayerController::BindToTargetingComponent()
{
	UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent();
	if (!TargetingComponent || BoundTargetingComponent.Get() == TargetingComponent)
	{
		return;
	}

	UnbindFromTargetingComponent();

	TargetingComponent->OnTargetingDisplayUpdated.AddUObject(this, &ACPlayerController::HandleTargetingDisplayUpdated);
	TargetingComponent->OnTargetingDisplayCleared.AddUObject(this, &ACPlayerController::HandleTargetingDisplayCleared);
	TargetingComponent->OnHardTargetChanged.AddUObject(this, &ACPlayerController::HandleHardTargetChanged);
	BoundTargetingComponent = TargetingComponent;

	HandleHardTargetChanged(TargetingComponent->GetHardTarget());
}

void ACPlayerController::UnbindFromTargetingComponent()
{
	if (UTargetingComponent* TargetingComponent = BoundTargetingComponent.Get())
	{
		TargetingComponent->OnTargetingDisplayUpdated.RemoveAll(this);
		TargetingComponent->OnTargetingDisplayCleared.RemoveAll(this);
		TargetingComponent->OnHardTargetChanged.RemoveAll(this);
	}

	BoundTargetingComponent.Reset();
}

void ACPlayerController::OnMoveInput(const FInputActionValue& Value)
{
	if (IsModalGameplayInputLocked() || IsCinematicMoveInputLocked())
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();

	ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMoveInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}
	
	PlayerCharacter->Move(MoveInput);
}

void ACPlayerController::OnJumpInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicMoveInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnJumpInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}

	if (PlayerCharacter->CanJump())
	{
		PlayerCharacter->Jump();
	}
}

void ACPlayerController::OnLookInput(const FInputActionValue& Value)
{
	if (IsModalGameplayInputLocked() || IsCinematicLookInputLocked())
	{
		return;
	}

	const FVector2D LookInput = Value.Get<FVector2D>();

	if (LookInput.IsNearlyZero())
	{
		return;
	}

	if (ACameraRigActor* CurrentCameraRig = EnsureCameraRig())
	{
		CurrentCameraRig->AddLookInput(LookInput);
	}
}

void ACPlayerController::OnCameraZoomInput(const FInputActionValue& Value)
{
	if (IsModalGameplayInputLocked() || IsCinematicLookInputLocked())
	{
		return;
	}
	
	const float AxisValue = Value.Get<float>();
	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}
		
	if (ACameraRigActor* CurrentCameraRig = EnsureCameraRig())
	{
		CurrentCameraRig->AdjustZoom(-AxisValue);
	}
}

void ACPlayerController::OnAttackInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAttackInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}

	PlayerCharacter->RequestAttack();
}

void ACPlayerController::OnDodgeInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDodgeInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}
	
	UPlayerMovementComponent* PlayerMovementComp = Cast<UPlayerMovementComponent>(PlayerCharacter->GetMovementComponent());
	if (!PlayerMovementComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnDodgeInput failed: MovementComponent is not UPlayerMovementComponent."));
		return;
	}
	
	if (PlayerMovementComp->CanDodge())
	{
		if (PlayerCharacter->RequestDodge())
		{
			PlayerMovementComp->ConsumeDodge();
		}
	}
}



void ACPlayerController::OnSkill1Input()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnSkill1Input failed: Pawn is not ACPlayerCharacter."));
		return;
	}

	PlayerCharacter->RequestSkill(Skill1Data);
}

void ACPlayerController::OnSkill2Input()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnSkill2Input failed: Pawn is not ACPlayerCharacter."));
		return;
	}

	PlayerCharacter->RequestSkill(Skill2Data);
}

void ACPlayerController::OnHardTargetInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->RequestHardTarget();
	}
}

void ACPlayerController::OnRangedHardTargetTriggered()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->BeginRangedHardTargetAim();
	}
}

void ACPlayerController::OnRangedHardTargetCompleted()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->CompleteRangedHardTargetAim();
	}
}

void ACPlayerController::OnRangedHardTargetCanceled()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->CancelRangedHardTargetAim();
	}
}

void ACPlayerController::OnClearHardTargetInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->ClearHardTarget();
	}
}

void ACPlayerController::OnPauseInput()
{
	if (HousingPlacementComponent && HousingPlacementComponent->IsHousingActive())
	{
		HousingPlacementComponent->ExitHousing();
		return;
	}

	if (bCommandBoxMenuOpen)
	{
		CloseCommandBoxMenu();
		return;
	}

	RequestTogglePause();
}

void ACPlayerController::OnInteractInput()
{
	if (IsModalGameplayInputLocked() || IsCinematicGameplayInputLocked())
	{
		return;
	}

	if (UInteractionComponent* InteractionComponent = GetPlayerInteractionComponent())
	{
		InteractionComponent->TryInteract();
	}
}

void ACPlayerController::RequestTogglePause()
{
	SetGameplayPaused(!UGameplayStatics::IsGamePaused(this));
}

void ACPlayerController::RequestResumeGame()
{
	SetGameplayPaused(false);
}

void ACPlayerController::OpenCommandBoxMenu(ACommandBoxActor* CommandBox)
{
	if (IsModalGameplayInputLocked() || !IsValid(CommandBox))
	{
		return;
	}

	ACPlayerHUD* PlayerHUD = Cast<ACPlayerHUD>(GetHUD());
	UCommandBoxMenuWidget* MenuWidget = PlayerHUD ? PlayerHUD->ShowCommandBoxMenu() : nullptr;
	if (!MenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("OpenCommandBoxMenu failed because CommandBoxMenuWidget is not available."));
		return;
	}

	ActiveCommandBox = CommandBox;
	bCommandBoxMenuOpen = true;
	if (UInteractionComponent* InteractionComponent = GetPlayerInteractionComponent())
	{
		InteractionComponent->SetInteractionPromptEnabled(false);
	}

	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetWidgetToFocus(MenuWidget->TakeWidget());
	SetInputMode(InputMode);
}

void ACPlayerController::CloseCommandBoxMenu()
{
	if (!bCommandBoxMenuOpen)
	{
		return;
	}

	if (ACPlayerHUD* PlayerHUD = Cast<ACPlayerHUD>(GetHUD()))
	{
		PlayerHUD->HideCommandBoxMenu();
	}

	bCommandBoxMenuOpen = false;
	ActiveCommandBox.Reset();

	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetInputMode(FInputModeGameOnly());

	if (UInteractionComponent* InteractionComponent = GetPlayerInteractionComponent())
	{
		InteractionComponent->SetInteractionPromptEnabled(true);
	}
}

void ACPlayerController::RequestCommandBoxHousing()
{
	ACommandBoxActor* CommandBox = ActiveCommandBox.Get();
	CloseCommandBoxMenu();
	if (IsValid(CommandBox))
	{
		IInteractable::Execute_SetInteractionPromptVisible(CommandBox, false);
		CommandBox->RequestHousing(this);
	}
}

void ACPlayerController::RequestCommandBoxCostume()
{
	ACommandBoxActor* CommandBox = ActiveCommandBox.Get();
	CloseCommandBoxMenu();
	if (IsValid(CommandBox))
	{
		IInteractable::Execute_SetInteractionPromptVisible(CommandBox, false);
		CommandBox->RequestCostume(this);
	}
}

void ACPlayerController::RequestCommandBoxStageTravel()
{
	ACommandBoxActor* CommandBox = ActiveCommandBox.Get();
	CloseCommandBoxMenu();
	if (IsValid(CommandBox))
	{
		IInteractable::Execute_SetInteractionPromptVisible(CommandBox, false);
		CommandBox->RequestStageTravel(this);
	}
}

void ACPlayerController::SetGameplayPaused(bool bShouldPause)
{
	ACPlayerHUD* PlayerHUD = Cast<ACPlayerHUD>(GetHUD());

	if (bShouldPause)
	{
		UPauseMenuWidget* PauseMenuWidget = PlayerHUD ? PlayerHUD->ShowPauseMenu() : nullptr;
		if (!PauseMenuWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetGameplayPaused failed because PauseMenuWidget is not available."));
			return;
		}

		if (!SetPause(true))
		{
			PlayerHUD->HidePauseMenu();
			UE_LOG(LogTemp, Warning, TEXT("SetGameplayPaused failed to pause the game."));
			return;
		}

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
		SetInputMode(InputMode);
		return;
	}

	if (!SetPause(false))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetGameplayPaused failed to resume the game."));
		return;
	}

	if (PlayerHUD)
	{
		PlayerHUD->HidePauseMenu();
	}

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	SetInputMode(FInputModeGameOnly());
}

void ACPlayerController::HandleTargetingDisplayUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers)
{
	if (ACPlayerHUD* PlayerHUD = Cast<ACPlayerHUD>(GetHUD()))
	{
		PlayerHUD->OnTargetingHUDUpdated(bShowCrosshair, Markers);
	}
}

void ACPlayerController::HandleTargetingDisplayCleared()
{
	if (ACPlayerHUD* PlayerHUD = Cast<ACPlayerHUD>(GetHUD()))
	{
		PlayerHUD->OnTargetingHUDCleared();
	}
}

void ACPlayerController::HandleHardTargetChanged(AActor* NewHardTarget)
{
	if (ACameraRigActor* CurrentCameraRig = EnsureCameraRig())
	{
		CurrentCameraRig->SetFocusTarget(NewHardTarget);
	}
}

ACPlayerCharacter* ACPlayerController::GetPlayerCharacter() const
{
	return Cast<ACPlayerCharacter>(GetPawn());
}

UTargetingComponent* ACPlayerController::GetPlayerTargetingComponent() const
{
	const ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter ? PlayerCharacter->GetTargetingComponent() : nullptr;
}

bool ACPlayerController::IsCinematicMoveInputLocked() const
{
	const UWorld* World = GetWorld();
	const UCinematicInputLockSubsystem* InputLockSubsystem = World ? World->GetSubsystem<UCinematicInputLockSubsystem>() : nullptr;
	return InputLockSubsystem && InputLockSubsystem->IsMoveInputLocked(this);
}

bool ACPlayerController::IsCinematicLookInputLocked() const
{
	const UWorld* World = GetWorld();
	const UCinematicInputLockSubsystem* InputLockSubsystem = World ? World->GetSubsystem<UCinematicInputLockSubsystem>() : nullptr;
	return InputLockSubsystem && InputLockSubsystem->IsLookInputLocked(this);
}

UInteractionComponent* ACPlayerController::GetPlayerInteractionComponent() const
{
	const ACPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
	return PlayerCharacter ? PlayerCharacter->GetInteractionComponent() : nullptr;
}

bool ACPlayerController::IsModalGameplayInputLocked() const
{
	return bCommandBoxMenuOpen || (HousingPlacementComponent && HousingPlacementComponent->IsHousingActive());
}

bool ACPlayerController::IsCinematicGameplayInputLocked() const
{
	const UWorld* World = GetWorld();
	const UCinematicInputLockSubsystem* InputLockSubsystem = World ? World->GetSubsystem<UCinematicInputLockSubsystem>() : nullptr;
	return InputLockSubsystem && InputLockSubsystem->IsGameplayInputLocked(this);
}

void ACPlayerController::AddDefaultInputMappingContext()
{
	if (!DefaultInputMappingContext || !GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
}

void ACPlayerController::RemoveDefaultInputMappingContext()
{
	if (!DefaultInputMappingContext || !GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->RemoveMappingContext(DefaultInputMappingContext);
	}
}

#if !UE_BUILD_SHIPPING

void ACPlayerController::ToggleDevConsole()
{
	if (!bIsDevConsoleOpen)
	{
		if (!DevConsoleWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("ToggleDevConsole: DevConsoleWidgetClass is not assigned in BP."));
			return;
		}

		DevConsoleWidget = CreateWidget<UUserWidget>(this, DevConsoleWidgetClass);
		if (DevConsoleWidget)
		{
			DevConsoleWidget->AddToViewport(100); // 높은 우선순위 오버레이
			bIsDevConsoleOpen = true;

			// 마우스 커서 표시
			SetShowMouseCursor(true);
			bEnableClickEvents = true;
			bEnableMouseOverEvents = true;

			// 입력 포커스는 UMG 위젯 내부에서 처리하도록 권장 (EditBox Focus)
			// 필요 시 여기에서 직접 SetInputModeGameAndUI 사용 가능.
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ToggleDevConsole: Failed to create DevConsoleWidget."));
		}
	}
	else
	{
		if (DevConsoleWidget)
		{
			DevConsoleWidget->RemoveFromParent();
			DevConsoleWidget = nullptr;
		}

		bIsDevConsoleOpen = false;

		// 마우스 커서 숨김 및 게임 입력 복구
		SetShowMouseCursor(false);
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
	}
}

#endif // !UE_BUILD_SHIPPING
