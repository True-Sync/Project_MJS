// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayerCharacter.generated.h"

class UAttackComponent;
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
	bool RequestAttack();
	bool RequestDodge();

	UAttackComponent* GetAttackComponent() const { return AttackComponent; }
	UDodgeComponent* GetDodgeComponent() const { return DodgeComponent; }

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttackComponent> AttackComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDodgeComponent> DodgeComponent;
};
