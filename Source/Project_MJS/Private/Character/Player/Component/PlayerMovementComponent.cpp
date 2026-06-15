// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Component/PlayerMovementComponent.h"

#include "TimerManager.h"


bool UPlayerMovementComponent::CanDodge() const
{
	if (CurrentDodgeCount >= MaxDodgeCount)
	{
		return false;
	}

	if (IsFalling())
	{
		return CurrentAirDodgeCount < MaxAirDodgeCount;
	}

	return true;
}

void UPlayerMovementComponent::ConsumeDodge()
{
	CurrentDodgeCount++;

	if (IsFalling())
	{
		CurrentAirDodgeCount++;
		return;
	}

	StartGroundDodgeCooldown();
}

void UPlayerMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (MovementMode == MOVE_Walking)
	{
		ResetDodgeCounts();
	}
}

void UPlayerMovementComponent::ResetDodgeCounts()
{
	CurrentDodgeCount = 0;
	CurrentAirDodgeCount = 0;
	bGroundDodgeOnCooldown = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(GroundDodgeCooldownTimerHandle);
	}
}

void UPlayerMovementComponent::StartGroundDodgeCooldown()
{
	if (bGroundDodgeOnCooldown)
	{
		return;
	}

	if (GroundDodgeCooldown <= 0.0f)
	{
		CurrentDodgeCount = 0;
		return;
	}

	bGroundDodgeOnCooldown = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		bGroundDodgeOnCooldown = false;
		return;
	}

	World->GetTimerManager().SetTimer(
		GroundDodgeCooldownTimerHandle,
		this,
		&UPlayerMovementComponent::EndGroundDodgeCooldown,
		GroundDodgeCooldown,
		false);
}

void UPlayerMovementComponent::EndGroundDodgeCooldown()
{
	CurrentDodgeCount = 0;
	bGroundDodgeOnCooldown = false;
}
