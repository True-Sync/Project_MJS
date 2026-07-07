// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CPlayerController.h"

#include "Camera/CameraRigActor.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/Player/CPlayerHUD.h"
#include "Character/Player/Component/PlayerMovementComponent.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

void ACPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (DefaultInputMappingContext && GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}

	InitializeCameraRig();
	BindToTargetingComponent();
}

void ACPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromTargetingComponent();
	Super::EndPlay(EndPlayReason);
}

void ACPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (CameraRig)
	{
		CameraRig->SetCameraTarget(InPawn);
	}

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

	if (IA_Skill2)
	{
		EnhancedInputComponent->BindAction(IA_Skill2, ETriggerEvent::Started, this, &ACPlayerController::OnSkill2Input);
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
}	

FRotator ACPlayerController::GetCameraYawRotation() const
{
	return CameraRig ? CameraRig->GetCameraYawRotation() : FRotator(0.0f, GetControlRotation().Yaw, 0.0f);
}

void ACPlayerController::InitializeCameraRig()
{
	if (CameraRig)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ACameraRigActor> It(World); It; ++It)
	{
		CameraRig = *It;
		break;
	}

	if (!CameraRig)
	{
		UClass* SpawnClass = CameraRigClass ? CameraRigClass.Get() : ACameraRigActor::StaticClass();
		const FVector RigSpawnLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
		CameraRig = World->SpawnActor<ACameraRigActor>(SpawnClass, RigSpawnLocation, FRotator::ZeroRotator);
	}

	if (CameraRig)
	{
		CameraRig->SetCameraTarget(GetPawn());
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
	if (IsCinematicMoveInputLocked())
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();

	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnMoveInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}
	
	PlayerCharacter->Move(MoveInput);
}

void ACPlayerController::OnJumpInput()
{
	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
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
	if (IsCinematicLookInputLocked())
	{
		return;
	}

	const FVector2D LookInput = Value.Get<FVector2D>();

	if (LookInput.IsNearlyZero())
	{
		return;
	}

	if (CameraRig)
	{
		CameraRig->AddLookInput(LookInput);
	}
}

void ACPlayerController::OnDodgeInput()
{
	if (IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
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

void ACPlayerController::OnAttackInput()
{
	if (IsCinematicGameplayInputLocked())
	{
		return;
	}

	ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
	
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnAttackInput failed: Pawn is not ACPlayerCharacter."));
		return;
	}

	PlayerCharacter->RequestAttack();
}


void ACPlayerController::OnSkill1Input()
{
	if (IsCinematicGameplayInputLocked())
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
	if (IsCinematicGameplayInputLocked())
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
	if (IsCinematicGameplayInputLocked())
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
	if (IsCinematicGameplayInputLocked())
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
	if (IsCinematicGameplayInputLocked())
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
	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->CancelRangedHardTargetAim();
	}
}

void ACPlayerController::OnClearHardTargetInput()
{
	if (UTargetingComponent* TargetingComponent = GetPlayerTargetingComponent())
	{
		TargetingComponent->ClearHardTarget();
	}
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
	if (CameraRig)
	{
		CameraRig->SetFocusTarget(NewHardTarget);
	}
}

UTargetingComponent* ACPlayerController::GetPlayerTargetingComponent() const
{
	const ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn());
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

bool ACPlayerController::IsCinematicGameplayInputLocked() const
{
	const UWorld* World = GetWorld();
	const UCinematicInputLockSubsystem* InputLockSubsystem = World ? World->GetSubsystem<UCinematicInputLockSubsystem>() : nullptr;
	return InputLockSubsystem && InputLockSubsystem->IsGameplayInputLocked(this);
}
