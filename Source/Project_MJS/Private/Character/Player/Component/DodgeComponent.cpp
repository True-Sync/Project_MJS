#include "Character/Player/Component/DodgeComponent.h"

#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraDirectingComponentFinder.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/SharedComponent/StaminaComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "System/VFX/VFXExcutorComponent.h"
#include "System/VFX/VFXGameplayTags.h"

UDodgeComponent::UDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	DodgeStaminaCost.StaminaCost = 20.0f;
}

bool UDodgeComponent::RequestDodge()
{
	if (bIsDodging)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge rejected: already dodging."));
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Owner is not ACharacter."));
		return false;
	}

	UStaminaComponent* StaminaComponent = OwnerCharacter->FindComponentByClass<UStaminaComponent>();
	if (!IsValid(StaminaComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: StaminaComponent is missing. Character=%s"), *GetNameSafe(OwnerCharacter));
		return false;
	}

	if (!StaminaComponent->CanConsumeStamina(DodgeStaminaCost))
	{
		UE_LOG(LogTemp, Log, TEXT("RequestDodge rejected: insufficient stamina. Current=%.1f Cost=%.1f"),
			StaminaComponent->GetCurrentStamina(), DodgeStaminaCost.StaminaCost);
		return false;
	}
	
	UVFXExcutorComponent* VFXExecuterComponent = OwnerCharacter->FindComponentByClass<UVFXExcutorComponent>();
	if(!IsValid(VFXExecuterComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("RequestDodge rejected: VFXExecuterComponent is Missing. Character = %s"), *GetNameSafe(OwnerCharacter));
		return false;
	}
	
	FVector DodgeDirection;
	const ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(OwnerCharacter);
	const bool bHasMoveInput = PlayerCharacter && PlayerCharacter->GetLastMoveWorldDirection(DodgeDirection);
	UAnimMontage* MontageToPlay = bHasMoveInput ? DefaultDodgeMontage.Get() : BackStepDodgeMontage.Get();
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: dodge montage is not assigned on %s."), *GetNameSafe(this));
		return false;
	}

	if (bHasMoveInput)
	{
		OwnerCharacter->SetActorRotation(DodgeDirection.Rotation());
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: AnimInstance is missing. Character=%s"), *GetNameSafe(OwnerCharacter));
		return false;
	}
	
	const float Duration = AnimInstance->Montage_Play(MontageToPlay);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Montage_Play returned 0. Check AnimBP slot setup. Montage=%s"), *GetNameSafe(MontageToPlay));
		return false;
	}
	
	StartDodgeLoopVFX(OwnerCharacter->GetActorLocation());
	
	if (!StaminaComponent->ConsumeStamina(DodgeStaminaCost))
	{
		AnimInstance->Montage_Stop(0.0f, MontageToPlay);
		return false;
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UDodgeComponent::OnDodgeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
	
	ActiveDodgeMontage = MontageToPlay;
	LastDodgeInputTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -999.0f;
	bJustDodgeConsumed = false;
	bIsDodging = true;
	return true;
}

bool UDodgeComponent::TryConsumeJustDodge(AActor* AttackCauser)
{
	if (!bEnableJustDodge || !bIsDodging || bJustDodgeConsumed)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float ElapsedAfterDodgeInput = World->GetTimeSeconds() - LastDodgeInputTime;
	if (ElapsedAfterDodgeInput < JustDodgeMinElapsed || ElapsedAfterDodgeInput > JustDodgeWindow)
	{
		return false;
	}

	bJustDodgeConsumed = true;
	HandleJustDodgeSucceeded(AttackCauser);
	return true;
}

bool UDodgeComponent::ConsumeJustDodgeCounter()
{
	if (!bCanJustDodgeCounter)
	{
		return false;
	}

	CloseJustDodgeCounterWindow();
	return true;
}

UVFXExcutorComponent* UDodgeComponent::ResolveVFXExcutor()
{
	if (IsValid(CachedVFXExecutor))
		return CachedVFXExecutor;
	
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
		return nullptr;
	
	CachedVFXExecutor = Owner->FindComponentByClass<UVFXExcutorComponent>();
	return CachedVFXExecutor;
}


//=============== VFX =================

FVFXExecuteContext UDodgeComponent::MakeDodgeVFXContext(const FVector& DodgeDirection)
{
	FVFXExecuteContext Context;

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return Context;
	}

	Context.SourceActor = Owner;
	Context.TargetActor = Owner;
	Context.WorldTransform = Owner->GetActorTransform();
	Context.AttachComponent = Owner->GetRootComponent();
	Context.Direction = DodgeDirection;

	return Context;
}

void UDodgeComponent::PlayDodgeStartVFX(const FVector& DodgeDirection)
{
	//OneShot 전용 -> 따로 핸들 필요 없음.
	if (UVFXExcutorComponent* VFXExcutor = ResolveVFXExcutor())
	{
		VFXExcutor->ExecuteVFX(ProjectVFXTags::Character_Dodge_Start,MakeDodgeVFXContext(DodgeDirection));
	}
}

void UDodgeComponent::StartDodgeLoopVFX(const FVector& DodgeDirection)
{
	//Loop 전용
	UVFXExcutorComponent* VFXExecutor = ResolveVFXExcutor();
	if (!IsValid(VFXExecutor))
		return;
	
	StopDodgeLoopVFX(); //기존 Loop 정리
	DodgeLoopVFXHandle = VFXExecutor->ExecuteVFX(ProjectVFXTags::Character_Dodge_Loop, MakeDodgeVFXContext(DodgeDirection));
}

void UDodgeComponent::StopDodgeLoopVFX()
{
	if (!DodgeLoopVFXHandle.IsValid())
		return;
	
	if (UVFXExcutorComponent* VFXExcutor = ResolveVFXExcutor())
		VFXExcutor->StopVFX(DodgeLoopVFXHandle);
	
	DodgeLoopVFXHandle.Reset();
}

void UDodgeComponent::FinishDodgeVFX()
{
	StopDodgeLoopVFX();
	AActor* Owner = GetOwner();
	
	UVFXExcutorComponent* VFXExecutor = ResolveVFXExcutor();
	if (!IsValid(VFXExecutor))
		VFXExecutor->ExecuteVFX(ProjectVFXTags::Character_Dodge_End,MakeDodgeVFXContext(Owner->GetActorLocation()));
}


void UDodgeComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveDodgeMontage)
	{
		return;
	}

	StopDodgeLoopVFX();
	bIsDodging = false;
	bJustDodgeConsumed = false;
	ActiveDodgeMontage = nullptr;
}

void UDodgeComponent::HandleJustDodgeSucceeded(AActor* AttackCauser)
{
	OpenJustDodgeCounterWindow();
	PlayJustDodgeCameraFeedback();
	ApplyJustDodgeTimeFeedback(AttackCauser);

	UE_LOG(LogTemp, Log, TEXT("Just dodge succeeded. Owner=%s AttackCauser=%s"), *GetNameSafe(GetOwner()), *GetNameSafe(AttackCauser));
}

void UDodgeComponent::PlayJustDodgeCameraFeedback() const
{
	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	const USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	UCameraDirectingComponent* CameraDirectingComponent = FindCameraDirectingComponentFromMesh(MeshComponent);
	if (!CameraDirectingComponent)
	{
		return;
	}

	if (bPlayJustDodgeCameraShake && JustDodgeCameraShake)
	{
		CameraDirectingComponent->PlayCameraShake(JustDodgeCameraShake, JustDodgeCameraShakeScale);
	}

	if (bPlayJustDodgeFOV)
	{
		CameraDirectingComponent->PlayFOV(JustDodgeFOV, JustDodgeFOVBlendInTime, JustDodgeFOVHoldTime, JustDodgeFOVBlendOutTime);
	}
}

void UDodgeComponent::ApplyJustDodgeTimeFeedback(AActor* AttackCauser)
{
	if (!bUseJustDodgeTimeFeedback)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UCombatTimeDilationSubsystem* TimeDilationSubsystem = World->GetSubsystem<UCombatTimeDilationSubsystem>();
	if (!TimeDilationSubsystem)
	{
		return;
	}

	if (JustDodgeTimeFeedbackMode == ECombatTimeDilationFeedbackMode::HitStop)
	{
		TimeDilationSubsystem->PlayHitStop(JustDodgeHitStopSettings, GetOwner(), AttackCauser);
	}
	else
	{
		TimeDilationSubsystem->PlayWorldSlow(JustDodgeWorldSlowSettings);
	}
}

void UDodgeComponent::OpenJustDodgeCounterWindow()
{
	if (!bEnableJustDodgeCounter)
	{
		return;
	}

	bCanJustDodgeCounter = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(JustDodgeCounterTimerHandle);
	if (JustDodgeCounterWindow <= 0.0f)
	{
		CloseJustDodgeCounterWindow();
		return;
	}

	World->GetTimerManager().SetTimer(
		JustDodgeCounterTimerHandle,
		this,
		&UDodgeComponent::CloseJustDodgeCounterWindow,
		JustDodgeCounterWindow,
		false);
}

void UDodgeComponent::CloseJustDodgeCounterWindow()
{
	bCanJustDodgeCounter = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(JustDodgeCounterTimerHandle);
	}
}
