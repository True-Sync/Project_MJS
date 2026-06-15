// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_MJS_API UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	bool CanDodge() const;
	void ConsumeDodge();
	
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	int32 MaxDodgeCount = 2;

	UPROPERTY(VisibleAnywhere, Category = "Dash")
	int32 CurrentDodgeCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	int32 MaxAirDodgeCount = 1;

	UPROPERTY(VisibleAnywhere, Category = "Dash")
	int32 CurrentAirDodgeCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float GroundDodgeCooldown = 1.0f;

	UPROPERTY(Transient)
	bool bGroundDodgeOnCooldown = false;

	UPROPERTY(Transient)
	FTimerHandle GroundDodgeCooldownTimerHandle;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	void ResetDodgeCounts();
	void StartGroundDodgeCooldown();
	void EndGroundDodgeCooldown();
};
