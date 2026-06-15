// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CPlayerController.h"

#include "Camera/CameraRigActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "InputActionValue.h"

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
}

void ACPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (CameraRig)
	{
		CameraRig->SetCameraTarget(InPawn);
	}
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

void ACPlayerController::OnMoveInput(const FInputActionValue& Value)
{
	if (IsCinematicMoveInputLocked())
	{
		return;
	}

	const FVector2D MoveInput = Value.Get<FVector2D>();

	if (ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->Move(MoveInput);
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
	
	PlayerCharacter->RequestDodge();
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
