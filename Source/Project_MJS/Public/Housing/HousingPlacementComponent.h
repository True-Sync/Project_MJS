#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Housing/HousingTypes.h"
#include "HousingPlacementComponent.generated.h"

class AActor;
class ACPlayerController;
class AHousingAreaActor;
class UActorComponent;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;
class UHousingItemDataAsset;
class UMaterialInterface;
class UMeshComponent;
class UPrimitiveComponent;
class USceneComponent;

struct FHousingPreviewMeshMaterialState
{
	TWeakObjectPtr<UMeshComponent> Component;
	int32 FirstMaterialIndex = 0;
	int32 NumMaterials = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHousingCatalogVisibilityChanged, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHousingStateChanged, EHousingPlacementState);

UCLASS(ClassGroup = (Housing), meta = (BlueprintSpawnableComponent))
class PROJECT_MJS_API UHousingPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHousingPlacementComponent();

	void InitializeInput(UEnhancedInputComponent* EnhancedInputComponent);

	UFUNCTION(BlueprintCallable, Category = "Housing")
	bool EnterHousing(AHousingAreaActor* HousingArea);

	UFUNCTION(BlueprintCallable, Category = "Housing")
	void ExitHousing();

	UFUNCTION(BlueprintCallable, Category = "Housing")
	void ToggleCatalog();

	UFUNCTION(BlueprintCallable, Category = "Housing|Placement")
	bool BeginPlacement(UHousingItemDataAsset* ItemData);

	UFUNCTION(BlueprintCallable, Category = "Housing|Placement")
	void CancelPlacement();

	UFUNCTION(BlueprintCallable, Category = "Housing|Placement")
	void RotatePlacementClockwise();

	UFUNCTION(BlueprintCallable, Category = "Housing|Placement")
	bool ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "Housing|Placement")
	void RecallPlacement();

	UFUNCTION(BlueprintPure, Category = "Housing")
	bool IsHousingActive() const { return State != EHousingPlacementState::Inactive; }

	UFUNCTION(BlueprintPure, Category = "Housing")
	EHousingPlacementState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Housing|Placement")
	bool IsItemAvailable(const UHousingItemDataAsset* ItemData) const;

	UFUNCTION(BlueprintPure, Category = "Housing|UI")
	EHousingGuideContext GetGuideContext() const;

	AHousingAreaActor* GetActiveArea() const { return ActiveArea.Get(); }

	FOnHousingCatalogVisibilityChanged OnCatalogVisibilityChanged;
	FOnHousingStateChanged OnStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void HandleExitInput();
	void HandleConfirmInput();
	void HandleSelectPlacedPropInput();
	void HandleCancelSelectPlacedPropInput();
	bool BeginMovePlacement(AActor* PlacedActor);
	void UpdatePreviewFromCursor();
	AActor* SpawnPreviewActor(UHousingItemDataAsset* ItemData);
	void PreparePreviewActor();
	void RestorePreviewActorState();
	void DestroyPreview();
	void ResetPlacementData();
	void ApplyPreviewMaterial(bool bPlacementValid);
	void SetState(EHousingPlacementState NewState);
	ACPlayerController* GetPlayerController() const;
	void AddHousingInputMapping();
	void RemoveHousingInputMapping();

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputMappingContext> HousingInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_ToggleCatalog;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_ExitHousing;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_RotatePlacement;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_ConfirmPlacement;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_SelectPlacedProp;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_CancelSelectPlacedProp;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	TObjectPtr<UInputAction> IA_RecallPlacement;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Input")
	int32 HousingInputPriority = 10;

	UPROPERTY(Transient)
	TWeakObjectPtr<AHousingAreaActor> ActiveArea;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> PreviousViewTarget;

	UPROPERTY(Transient)
	TObjectPtr<UHousingItemDataAsset> SelectedItem;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PreviewActor;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Preview")
	TObjectPtr<UMaterialInterface> ValidPreviewMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Preview")
	TObjectPtr<UMaterialInterface> InvalidPreviewMaterial;

	FHousingCellCoord CurrentAnchorCell;
	FTransform CurrentPlacementTransform = FTransform::Identity;
	TArray<FHousingCellCoord> CurrentOccupiedCells;
	uint8 CurrentRotationQuarterTurns = 0;
	bool bCurrentPlacementValid = false;
	bool bLastAppliedPreviewValidity = false;
	bool bMovingExistingPlacement = false;
	FHousingPlacementRecord OriginalPlacementRecord;
	TArray<FHousingCellCoord> OriginalOccupiedCells;
	FTransform OriginalActorTransform = FTransform::Identity;
	bool bPreviewActorTickEnabled = false;
	bool bPreviewActorCollisionEnabled = false;
	TMap<TWeakObjectPtr<UActorComponent>, bool> PreviewComponentTickStates;
	TMap<TWeakObjectPtr<USceneComponent>, TEnumAsByte<EComponentMobility::Type>> PreviewMobilityStates;
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TEnumAsByte<ECollisionEnabled::Type>> PreviewCollisionStates;
	TArray<FHousingPreviewMeshMaterialState> PreviewMeshMaterialStates;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> PreviewOriginalMaterials;

	UPROPERTY(VisibleInstanceOnly, Category = "Housing")
	EHousingPlacementState State = EHousingPlacementState::Inactive;

	bool bInputMappingAdded = false;
};
