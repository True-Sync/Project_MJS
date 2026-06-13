#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DodgeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_MJS_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDodgeComponent();

	void RequestDodge();
	
protected:
	virtual void BeginPlay() override;
	
private:
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DefaultDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> BackStepDodgeMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveDodgeMontage;

	bool bIsDodging = false;
};
