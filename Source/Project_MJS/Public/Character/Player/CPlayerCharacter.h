// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayerCharacter.generated.h"

class UAttackComponent;
class UCinematicActionComponent;
class UCinematicParticipantComponent;
class UDodgeComponent;

UCLASS()
class PROJECT_MJS_API ACPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACPlayerCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	void Move(const FVector2D& MoveInput);
	void RequestAttack();
	void RequestDodge();
	bool GetLastMoveWorldDirection(FVector& OutDirection) const;

	UAttackComponent* GetAttackComponent() const { return AttackComponent; }
	UDodgeComponent* GetDodgeComponent() const { return DodgeComponent; }
	UCinematicActionComponent* GetCinematicActionComponent() const { return CinematicActionComponent; }
	UCinematicParticipantComponent* GetCinematicParticipantComponent() const { return CinematicParticipantComponent; }

private:
	UPROPERTY(Transient)
	FVector LastMoveWorldDirection = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasMoveInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDodgeComponent> DodgeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicActionComponent> CinematicActionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicParticipantComponent> CinematicParticipantComponent;
};
