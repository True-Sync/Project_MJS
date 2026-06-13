#include "Character/Player/Component/DodgeComponent.h"

#include "GameFramework/Character.h"

UDodgeComponent::UDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDodgeComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UDodgeComponent::RequestDodge()
{
	if (!DodgeMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: DodgeMontage is not assigned on %s."), *GetNameSafe(this));
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Owner is not ACharacter."));
		return false;
	}

	const float Duration = OwnerCharacter->PlayAnimMontage(DodgeMontage);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestDodge failed: Montage_Play returned 0. Check AnimBP slot setup. Montage=%s"), *GetNameSafe(DodgeMontage));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("RequestDodge succeeded: Montage=%s Duration=%.2f"), *GetNameSafe(DodgeMontage), Duration);
	return true;
}

