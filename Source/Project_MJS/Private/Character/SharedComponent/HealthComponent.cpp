#include "Character/SharedComponent/HealthComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentHealth <= 0.0f || CurrentHealth > MaxHealth)
	{
		CurrentHealth = MaxHealth;
	}
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
}

void UHealthComponent::ConfigureDeathBehavior(bool bInDisableInputOnDeath, bool bInDestroyOwnerOnDeath)
{
	bDisableInputOnDeath = bInDisableInputOnDeath;
	bDestroyOwnerOnDeath = bInDestroyOwnerOnDeath;
}

bool UHealthComponent::ReceiveDamage(float DamageAmount, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return false;
	}

	ApplyHealthDelta(-DamageAmount, InstigatedBy, DamageCauser);
	return true;
}

bool UHealthComponent::Heal(float HealAmount)
{
	if (bIsDead || HealAmount <= 0.0f)
	{
		return false;
	}

	ApplyHealthDelta(HealAmount, nullptr, nullptr);
	return true;
}

void UHealthComponent::ApplyHealthDelta(float Delta, AController* InstigatedBy, AActor* DamageCauser)
{
	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Delta, 0.0f, MaxHealth);

	if (!FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		OnHealthChanged.Broadcast(OldHealth, CurrentHealth);
	}

	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		HandleDeath(InstigatedBy, DamageCauser);
	}
}

void UHealthComponent::HandleDeath(AController* InstigatedBy, AActor* DamageCauser)
{
	bIsDead = true;
	OnDeath.Broadcast();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (bDisableInputOnDeath)
	{
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			if (AController* OwnerController = OwnerPawn->GetController())
			{
				OwnerPawn->DisableInput(Cast<APlayerController>(OwnerController));
			}
		}
	}

	if (bDestroyOwnerOnDeath)
	{
		Owner->Destroy();
	}
}
