#include "Character/Player/Component/AttackComponent.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/Player/Component/DodgeComponent.h"
#include "Character/Player/Component/PlayerMovementComponent.h"
#include "Character/Player/Data/ComboAttackDataAsset.h"
#include "GameFramework/Character.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttackComponent::RequestAttack()
{
	if (const ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(GetOwner()))
	{
		const UDodgeComponent* DodgeComponent = PlayerCharacter->GetDodgeComponent();
		if (DodgeComponent && DodgeComponent->IsDodging())
		{
			UE_LOG(LogTemp, Log, TEXT("RequestAttack rejected: cannot attack while dodging."));
			return;
		}
	}

	if (bIsAttacking)
	{
		if (HasNextCombo())
		{
			bComboQueued = true;
			UE_LOG(LogTemp, Log, TEXT("RequestAttack: queued next combo. WindowOpen=%s CurrentComboIndex=%d"), bCanQueueCombo ? TEXT("true") : TEXT("false"), CurrentComboIndex);
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("RequestAttack rejected: already attacking and next combo does not exist. CurrentComboIndex=%d"), CurrentComboIndex);
		return;
	}

	PlayCombo(0);
}

void UAttackComponent::SetComboWindowOpen(bool bOpen)
{
	bCanQueueCombo = bOpen;
	UE_LOG(LogTemp, Log, TEXT("ComboWindow %s. CurrentComboIndex=%d Queued=%s"), bCanQueueCombo ? TEXT("opened") : TEXT("closed"), CurrentComboIndex, bComboQueued ? TEXT("true") : TEXT("false"));

	if (!bCanQueueCombo && bComboQueued)
	{
		PlayQueuedCombo();
	}
}

bool UAttackComponent::PlayCombo(int32 ComboIndex)
{
	if (!DA_BasicComboAttack || !DA_BasicComboAttack->ComboEntries.IsValidIndex(ComboIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: combo data is missing or index is invalid. DataAsset=%s Index=%d"), *GetNameSafe(DA_BasicComboAttack), ComboIndex);
		return false;
	}

	const FComboAttackEntry& ComboEntry = DA_BasicComboAttack->ComboEntries[ComboIndex];
	if (!ComboEntry.Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Montage is missing. DataAsset=%s Index=%d"), *GetNameSafe(DA_BasicComboAttack), ComboIndex);
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Owner is not ACharacter."));
		return false;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: AnimInstance is missing. Character=%s"), *GetNameSafe(OwnerCharacter));
		return false;
	}

	if (ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(OwnerCharacter))
	{
		FVector AttackDirection;
		if (PlayerCharacter->GetLastMoveWorldDirection(AttackDirection))
		{
			OwnerCharacter->SetActorRotation(AttackDirection.Rotation());
		}
	}

	const float Duration = AnimInstance->Montage_Play(ComboEntry.Montage);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Montage_Play returned 0. Check AnimBP slot setup. Montage=%s"), *GetNameSafe(ComboEntry.Montage));
		return false;
	}

	if (ComboEntry.SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(ComboEntry.SectionName, ComboEntry.Montage);
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UAttackComponent::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ComboEntry.Montage);

	CurrentComboIndex = ComboIndex;
	ActiveMontage = ComboEntry.Montage;
	bIsAttacking = true;
	bCanQueueCombo = false;
	bComboQueued = false;

	UE_LOG(LogTemp, Log, TEXT("PlayCombo succeeded: Index=%d Montage=%s Section=%s Duration=%.2f"), ComboIndex, *GetNameSafe(ComboEntry.Montage), *ComboEntry.SectionName.ToString(), Duration);
	return true;
}

void UAttackComponent::PlayQueuedCombo()
{
	const int32 NextComboIndex = CurrentComboIndex + 1;
	if (!PlayCombo(NextComboIndex))
	{
		bComboQueued = false;
	}
}

bool UAttackComponent::HasNextCombo() const
{
	return DA_BasicComboAttack && DA_BasicComboAttack->ComboEntries.IsValidIndex(CurrentComboIndex + 1);
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveMontage)
	{
		return;
	}

	bIsAttacking = false;
	bCanQueueCombo = false;
	bComboQueued = false;
	CurrentComboIndex = INDEX_NONE;
	ActiveMontage = nullptr;
}

