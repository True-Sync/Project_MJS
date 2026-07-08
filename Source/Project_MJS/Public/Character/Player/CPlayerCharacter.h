// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicParticipant.h"
#include "GameFramework/Character.h"
#include "CPlayerCharacter.generated.h"

class UPlayerMovementComponent;
class UAttackComponent;
class UCinematicActionComponent;
class UCinematicParticipantComponent;
class UDodgeComponent;
class UTargetingComponent;

UCLASS()
class PROJECT_MJS_API ACPlayerCharacter : public ACharacter, public ICinematicParticipant
{
	GENERATED_BODY()

public:
	ACPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;
	
public:
	void Move(const FVector2D& MoveInput);
	void RequestAttack();
	bool RequestDodge();

	// ===== Get/Setter =====
	bool GetLastMoveWorldDirection(FVector& OutDirection) const;
	UAttackComponent* GetAttackComponent() const { return AttackComponent; }
	UDodgeComponent* GetDodgeComponent() const { return DodgeComponent; }
	UTargetingComponent* GetTargetingComponent() const { return TargetingComponent; }
	UCinematicActionComponent* GetCinematicActionComponent() const { return CinematicActionComponent; }
	UCinematicParticipantComponent* GetCinematicParticipantComponent() const { return CinematicParticipantComponent; }

private:
	UPROPERTY(Transient)
	FVector LastMoveWorldDirection = FVector::ZeroVector;

	UPROPERTY(Transient)
	bool bHasMoveInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerMovementComponent> PlayerMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDodgeComponent> DodgeComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Targeting", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTargetingComponent> TargetingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicActionComponent> CinematicActionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component|Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicParticipantComponent> CinematicParticipantComponent;
};
