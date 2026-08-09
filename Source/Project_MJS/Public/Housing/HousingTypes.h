#pragma once

#include "CoreMinimal.h"
#include "HousingTypes.generated.h"

UENUM(BlueprintType)
enum class EHousingPlacementState : uint8
{
	Inactive,
	Browsing,
	CatalogOpen,
	Previewing,
	Saving,
	Error
};

UENUM(BlueprintType)
enum class EHousingGuideContext : uint8
{
	Hidden,
	Browsing,
	CatalogOpen,
	NewPlacement,
	MovingPlacedProp
};

UENUM(BlueprintType)
enum class EHousingItemCategory : uint8
{
	All,
	Furniture,
	Decoration,
	Lighting,
	Etc
};

USTRUCT(BlueprintType)
struct PROJECT_MJS_API FHousingCellCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FName RegionId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	int32 X = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	int32 Y = INDEX_NONE;

	bool IsValid() const
	{
		return !RegionId.IsNone() && X >= 0 && Y >= 0;
	}

	bool operator==(const FHousingCellCoord& Other) const
	{
		return RegionId == Other.RegionId && X == Other.X && Y == Other.Y;
	}
};

FORCEINLINE uint32 GetTypeHash(const FHousingCellCoord& Cell)
{
	uint32 Hash = GetTypeHash(Cell.RegionId);
	Hash = HashCombine(Hash, GetTypeHash(Cell.X));
	return HashCombine(Hash, GetTypeHash(Cell.Y));
}

USTRUCT(BlueprintType)
struct PROJECT_MJS_API FHousingPlacementRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FGuid PlacementId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FName AreaId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FHousingCellCoord AnchorCell;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing")
	FPrimaryAssetId ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing", meta = (ClampMin = "0", ClampMax = "3"))
	uint8 RotationQuarterTurns = 0;
};
