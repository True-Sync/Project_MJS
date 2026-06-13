#include "Character/Player/Component/DodgeComponent.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/CPlayerCharacter.h"
#include "GameFramework/Character.h"

UDodgeComponent::UDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDodgeComponent::RequestDodge()
{
	if (bIsDodging)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge rejected: already dodging."));
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Owner is not ACharacter."));
		return;
	}

	FVector DodgeDirection;
	const bool bHasMoveInput = Cast<ACPlayerCharacter>(OwnerCharacter) && Cast<ACPlayerCharacter>(OwnerCharacter)->GetLastMoveWorldDirection(DodgeDirection);
	UAnimMontage* MontageToPlay = bHasMoveInput ? DefaultDodgeMontage.Get() : BackStepDodgeMontage.Get();
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: dodge montage is not assigned on %s."), *GetNameSafe(this));
		return;
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
		return;
	}

	const float Duration = AnimInstance->Montage_Play(MontageToPlay);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Montage_Play returned 0. Check AnimBP slot setup. Montage=%s"), *GetNameSafe(MontageToPlay));
		return;
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UDodgeComponent::OnDodgeMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
	
	ActiveDodgeMontage = MontageToPlay;
	bIsDodging = true;
}

void UDodgeComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveDodgeMontage)
	{
		return;
	}

	bIsDodging = false;
	ActiveDodgeMontage = nullptr;
}
