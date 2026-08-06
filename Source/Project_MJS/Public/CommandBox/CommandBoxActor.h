#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/Interactable.h"
#include "CommandBoxActor.generated.h"

class ACPlayerController;
class ACommandBoxActor;
class UBoxComponent;
class UPrimitiveComponent;
class UInteractionComponent;
class UStaticMeshComponent;
class UWidgetComponent;
struct FHitResult;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FCommandBoxFeatureRequestedSignature,
	ACommandBoxActor*,
	ACPlayerController*);

UCLASS()
class PROJECT_MJS_API ACommandBoxActor : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ACommandBoxActor();

	virtual bool CanInteract_Implementation(APawn* InteractingPawn) const override;
	virtual void Interact_Implementation(APawn* InteractingPawn) override;
	virtual void SetInteractionPromptVisible_Implementation(bool bVisible) override;

	void RequestHousing(ACPlayerController* PlayerController);
	void RequestCostume(ACPlayerController* PlayerController);
	void RequestStageTravel(ACPlayerController* PlayerController);

	FCommandBoxFeatureRequestedSignature OnHousingRequested;

	FCommandBoxFeatureRequestedSignature OnCostumeRequested;

	FCommandBoxFeatureRequestedSignature OnStageTravelRequested;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleInteractionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void HandleInteractionEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	ACPlayerController* ResolvePlayerController(const APawn* Pawn) const;
	UInteractionComponent* ResolveInteractionComponent(AActor* OtherActor) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CommandBoxMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionVolume;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWidgetComponent> InteractionPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Command Box|Interaction", meta = (AllowPrivateAccess = "true"))
	bool bInteractionEnabled = true;

};
