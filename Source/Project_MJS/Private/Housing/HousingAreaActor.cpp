#include "Housing/HousingAreaActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Housing/HousingCameraActor.h"
#include "Housing/HousingGridRegionComponent.h"
#include "Housing/HousingItemDataAsset.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AHousingAreaActor::AHousingAreaActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(SceneRoot);

	DefaultGridRegion = CreateDefaultSubobject<UHousingGridRegionComponent>(TEXT("DefaultGridRegion"));
	DefaultGridRegion->SetupAttachment(SceneRoot);

	GridVisualization = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridVisualization"));
	GridVisualization->SetupAttachment(SceneRoot);
	GridVisualization->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GridVisualization->SetGenerateOverlapEvents(false);
	GridVisualization->SetCastShadow(false);
	GridVisualization->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		GridVisualization->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void AHousingAreaActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildGridVisualization();

	const UWorld* World = GetWorld();
	const bool bIsEditorWorld = !World || !World->IsGameWorld();
	GridVisualization->SetVisibility(bIsEditorWorld && bShowGridInEditor);
}

void AHousingAreaActor::BeginPlay()
{
	Super::BeginPlay();
	RebuildGridVisualization();
	SetGridVisible(false);
}

void AHousingAreaActor::SetGridVisible(bool bVisible)
{
	if (GridVisualization)
	{
		GridVisualization->SetVisibility(bVisible, true);
	}
}

void AHousingAreaActor::RebuildGridVisualization()
{
	if (!GridVisualization || !GridVisualization->GetStaticMesh())
	{
		return;
	}

	GridVisualization->ClearInstances();
	if (GridMaterial)
	{
		GridVisualization->SetMaterial(0, GridMaterial);
	}

	TArray<UHousingGridRegionComponent*> Regions;
	GetComponents(Regions);
	for (const UHousingGridRegionComponent* Region : Regions)
	{
		if (!Region || !Region->IsRegionEnabled())
		{
			continue;
		}

		const float CellScale = Region->GetCellSize() * CellFillRatio / 100.0f;
		const FVector InstanceScale(CellScale, CellScale, GridThickness / 100.0f);
		for (int32 Y = 0; Y < Region->GetRows(); ++Y)
		{
			for (int32 X = 0; X < Region->GetColumns(); ++X)
			{
				FVector CellCenter = Region->GetCellCenterWorld(X, Y);
				CellCenter += Region->GetUpVector() * GridHeightOffset;
				const FTransform InstanceTransform(Region->GetComponentQuat(), CellCenter, InstanceScale);
				GridVisualization->AddInstance(InstanceTransform, true);
			}
		}
	}
}

bool AHousingAreaActor::FindCellFromRay(
	const FVector& RayOrigin,
	const FVector& RayDirection,
	FHousingCellCoord& OutCell,
	FTransform& OutCellTransform) const
{
	OutCell = FHousingCellCoord();
	OutCellTransform = FTransform::Identity;

	float ClosestRayDistance = TNumericLimits<float>::Max();
	TArray<UHousingGridRegionComponent*> Regions;
	GetComponents(Regions);
	for (const UHousingGridRegionComponent* Region : Regions)
	{
		if (!Region || !Region->IsRegionEnabled())
		{
			continue;
		}

		const FVector PlaneNormal = Region->GetUpVector();
		const float Denominator = FVector::DotProduct(RayDirection, PlaneNormal);
		if (FMath::IsNearlyZero(Denominator))
		{
			continue;
		}

		const float RayDistance = FVector::DotProduct(Region->GetComponentLocation() - RayOrigin, PlaneNormal) / Denominator;
		if (RayDistance < 0.0f || RayDistance >= ClosestRayDistance)
		{
			continue;
		}

		const FVector PlaneHit = RayOrigin + RayDirection * RayDistance;
		int32 CellX = INDEX_NONE;
		int32 CellY = INDEX_NONE;
		if (!Region->WorldToCell(PlaneHit, CellX, CellY))
		{
			continue;
		}

		ClosestRayDistance = RayDistance;
		OutCell.RegionId = Region->GetRegionId();
		OutCell.X = CellX;
		OutCell.Y = CellY;
		OutCellTransform = FTransform(Region->GetComponentQuat(), Region->GetCellCenterWorld(CellX, CellY));
	}

	return OutCell.IsValid();
}

bool AHousingAreaActor::GetCellTransform(const FHousingCellCoord& Cell, FTransform& OutTransform) const
{
	const UHousingGridRegionComponent* Region = FindRegion(Cell.RegionId);
	if (!Region || !Region->IsValidCell(Cell.X, Cell.Y))
	{
		return false;
	}

	OutTransform = FTransform(Region->GetComponentQuat(), Region->GetCellCenterWorld(Cell.X, Cell.Y));
	return true;
}

bool AHousingAreaActor::BuildPlacement(
	const FHousingCellCoord& AnchorCell,
	FIntPoint Footprint,
	uint8 RotationQuarterTurns,
	TArray<FHousingCellCoord>& OutOccupiedCells,
	FTransform& OutPlacementTransform) const
{
	OutOccupiedCells.Reset();
	OutPlacementTransform = FTransform::Identity;

	const UHousingGridRegionComponent* Region = FindRegion(AnchorCell.RegionId);
	if (!Region || Footprint.X < 1 || Footprint.Y < 1)
	{
		return false;
	}

	const uint8 NormalizedRotation = RotationQuarterTurns % 4;
	const int32 Width = NormalizedRotation % 2 == 0 ? Footprint.X : Footprint.Y;
	const int32 Height = NormalizedRotation % 2 == 0 ? Footprint.Y : Footprint.X;
	const int32 MinX = AnchorCell.X - Width / 2;
	const int32 MinY = AnchorCell.Y - Height / 2;

	const float CellSize = Region->GetCellSize();
	FVector Center = Region->GetCellCenterWorld(AnchorCell.X, AnchorCell.Y);
	if (Width % 2 == 0)
	{
		Center -= Region->GetForwardVector() * (CellSize * 0.5f);
	}
	if (Height % 2 == 0)
	{
		Center -= Region->GetRightVector() * (CellSize * 0.5f);
	}

	const FQuat LocalYaw(FVector::UpVector, FMath::DegreesToRadians(90.0f * NormalizedRotation));
	OutPlacementTransform = FTransform(Region->GetComponentQuat() * LocalYaw, Center);

	bool bCanPlace = true;

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			FHousingCellCoord Cell;
			Cell.RegionId = AnchorCell.RegionId;
			Cell.X = MinX + X;
			Cell.Y = MinY + Y;
			if (!Region->IsValidCell(Cell.X, Cell.Y) || OccupiedCells.Contains(Cell))
			{
				bCanPlace = false;
			}
			OutOccupiedCells.Add(Cell);
		}
	}

	return bCanPlace;
}

bool AHousingAreaActor::RegisterPlacement(
	const FHousingPlacementRecord& PlacementRecord,
	const TArray<FHousingCellCoord>& InOccupiedCells,
	AActor* PlacedActor,
	UHousingItemDataAsset* ItemData)
{
	if (!IsValid(PlacedActor) || !IsValid(ItemData) || InOccupiedCells.IsEmpty() || IsRegisteredPlacement(PlacedActor))
	{
		return false;
	}

	for (const FHousingCellCoord& Cell : InOccupiedCells)
	{
		if (!Cell.IsValid() || OccupiedCells.Contains(Cell))
		{
			return false;
		}
	}

	OccupiedCells.Append(InOccupiedCells);
	PlacementRecords.Add(PlacementRecord);
	PlacedActors.Add(PlacedActor);

	FHousingRuntimePlacement& RuntimePlacement = RuntimePlacements.AddDefaulted_GetRef();
	RuntimePlacement.Record = PlacementRecord;
	RuntimePlacement.OccupiedCells = InOccupiedCells;
	RuntimePlacement.Actor = PlacedActor;
	RuntimePlacement.ItemData = ItemData;
	return true;
}

bool AHousingAreaActor::ReleasePlacement(
	AActor* PlacedActor,
	FHousingPlacementRecord& OutPlacementRecord,
	TArray<FHousingCellCoord>& OutOccupiedCells,
	UHousingItemDataAsset*& OutItemData)
{
	OutPlacementRecord = FHousingPlacementRecord();
	OutOccupiedCells.Reset();
	OutItemData = nullptr;

	const int32 RuntimeIndex = RuntimePlacements.IndexOfByPredicate([PlacedActor](const FHousingRuntimePlacement& Placement)
	{
		return Placement.Actor.Get() == PlacedActor;
	});
	if (RuntimeIndex == INDEX_NONE)
	{
		return false;
	}

	const FHousingRuntimePlacement RuntimePlacement = RuntimePlacements[RuntimeIndex];
	if (!RuntimePlacement.ItemData.IsValid())
	{
		return false;
	}

	OutPlacementRecord = RuntimePlacement.Record;
	OutOccupiedCells = RuntimePlacement.OccupiedCells;
	OutItemData = RuntimePlacement.ItemData.Get();

	for (const FHousingCellCoord& Cell : RuntimePlacement.OccupiedCells)
	{
		OccupiedCells.Remove(Cell);
	}

	PlacementRecords.RemoveAll([&RuntimePlacement](const FHousingPlacementRecord& Record)
	{
		return Record.PlacementId == RuntimePlacement.Record.PlacementId;
	});
	PlacedActors.Remove(PlacedActor);
	RuntimePlacements.RemoveAtSwap(RuntimeIndex);
	return IsValid(OutItemData);
}

bool AHousingAreaActor::IsRegisteredPlacement(const AActor* Actor) const
{
	return Actor && RuntimePlacements.ContainsByPredicate([Actor](const FHousingRuntimePlacement& Placement)
	{
		return Placement.Actor.Get() == Actor;
	});
}

bool AHousingAreaActor::IsItemPlaced(const UHousingItemDataAsset* ItemData) const
{
	if (!IsValid(ItemData))
	{
		return false;
	}

	const FPrimaryAssetId ItemAssetId = ItemData->GetPrimaryAssetId();
	return RuntimePlacements.ContainsByPredicate([ItemAssetId](const FHousingRuntimePlacement& Placement)
	{
		return Placement.Record.ItemId == ItemAssetId;
	});
}

UHousingGridRegionComponent* AHousingAreaActor::FindRegion(FName RegionId) const
{
	TArray<UHousingGridRegionComponent*> Regions;
	GetComponents(Regions);
	for (UHousingGridRegionComponent* Region : Regions)
	{
		if (Region && Region->GetRegionId() == RegionId)
		{
			return Region;
		}
	}

	return nullptr;
}
