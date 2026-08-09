#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Housing/HousingTypes.h"
#include "HousingAreaActor.generated.h"

class AHousingCameraActor;
class UHousingGridRegionComponent;
class UHousingItemDataAsset;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;

struct FHousingRuntimePlacement
{
	FHousingPlacementRecord Record;
	TArray<FHousingCellCoord> OccupiedCells;
	TWeakObjectPtr<AActor> Actor;
	TWeakObjectPtr<UHousingItemDataAsset> ItemData;
};

UCLASS()
class PROJECT_MJS_API AHousingAreaActor : public AActor
{
	GENERATED_BODY()

public:
	AHousingAreaActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Housing|Grid")
	void SetGridVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Housing|Grid")
	void RebuildGridVisualization();

	bool FindCellFromRay(
		const FVector& RayOrigin,
		const FVector& RayDirection,
		FHousingCellCoord& OutCell,
		FTransform& OutCellTransform) const;

	bool GetCellTransform(const FHousingCellCoord& Cell, FTransform& OutTransform) const;
	bool BuildPlacement(
		const FHousingCellCoord& AnchorCell,
		FIntPoint Footprint,
		uint8 RotationQuarterTurns,
		TArray<FHousingCellCoord>& OutOccupiedCells,
		FTransform& OutPlacementTransform) const;
	bool RegisterPlacement(
		const FHousingPlacementRecord& PlacementRecord,
		const TArray<FHousingCellCoord>& OccupiedCells,
		AActor* PlacedActor,
		UHousingItemDataAsset* ItemData);
	bool ReleasePlacement(
		AActor* PlacedActor,
		FHousingPlacementRecord& OutPlacementRecord,
		TArray<FHousingCellCoord>& OutOccupiedCells,
		UHousingItemDataAsset*& OutItemData);
	bool IsRegisteredPlacement(const AActor* Actor) const;
	bool IsItemPlaced(const UHousingItemDataAsset* ItemData) const;

	FName GetHousingAreaId() const { return HousingAreaId; }
	AHousingCameraActor* GetHousingCamera() const { return HousingCamera; }

private:
	UHousingGridRegionComponent* FindRegion(FName RegionId) const;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHousingGridRegionComponent> DefaultGridRegion;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UInstancedStaticMeshComponent> GridVisualization;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Housing|Area", meta = (AllowPrivateAccess = "true"))
	FName HousingAreaId = TEXT("HubHousingArea");

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Housing|Area", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AHousingCameraActor> HousingCamera;

	UPROPERTY(EditAnywhere, Category = "Housing|Visualization")
	TObjectPtr<UMaterialInterface> GridMaterial;

	UPROPERTY(EditAnywhere, Category = "Housing|Visualization", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float CellFillRatio = 0.94f;

	UPROPERTY(EditAnywhere, Category = "Housing|Visualization", meta = (ClampMin = "0.1"))
	float GridThickness = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Housing|Visualization")
	float GridHeightOffset = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Housing|Visualization")
	bool bShowGridInEditor = true;

	UPROPERTY(Transient)
	TSet<FHousingCellCoord> OccupiedCells;

	UPROPERTY(Transient)
	TArray<FHousingPlacementRecord> PlacementRecords;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> PlacedActors;

	TArray<FHousingRuntimePlacement> RuntimePlacements;
};
