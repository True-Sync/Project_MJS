#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Housing/HousingTypes.h"
#include "HousingItemDataAsset.generated.h"

class UStaticMesh;
class UTexture2D;

UCLASS(BlueprintType)
class PROJECT_MJS_API UHousingItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Display")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Display", meta = (MultiLine = "true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Display")
	EHousingItemCategory Category = EHousingItemCategory::Furniture;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Display")
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Display")
	int32 SortOrder = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item")
	TSoftClassPtr<AActor> PlacedActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item")
	TSoftObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item", meta = (ClampMin = "1"))
	FIntPoint Footprint = FIntPoint(1, 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item")
	bool bCanRotate = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Item")
	FVector PlacementOffset = FVector::ZeroVector;
};
