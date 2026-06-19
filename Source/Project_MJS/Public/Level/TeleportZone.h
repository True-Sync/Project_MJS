#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportZone.generated.h"

class UBoxComponent;

UCLASS()
class PROJECT_MJS_API ATeleportZone : public AActor
{
	GENERATED_BODY()

public:
	ATeleportZone();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool TeleportPawn(AActor* TargetActor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Teleport", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> TeleportTargetActor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport", meta = (AllowPrivateAccess = "true"))
	bool bUseTargetRotation = true;
};
