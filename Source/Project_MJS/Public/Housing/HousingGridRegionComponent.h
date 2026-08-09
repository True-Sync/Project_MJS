#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HousingGridRegionComponent.generated.h"

UCLASS(ClassGroup = (Housing), meta = (BlueprintSpawnableComponent))
class PROJECT_MJS_API UHousingGridRegionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UHousingGridRegionComponent();

	UFUNCTION(BlueprintPure, Category = "Housing|Grid")
	bool IsValidCell(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Housing|Grid")
	bool WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutY) const;

	UFUNCTION(BlueprintPure, Category = "Housing|Grid")
	FVector GetCellCenterWorld(int32 X, int32 Y) const;

	FName GetRegionId() const { return RegionId; }
	int32 GetColumns() const { return Columns; }
	int32 GetRows() const { return Rows; }
	float GetCellSize() const { return CellSize; }
	bool IsRegionEnabled() const { return bEnabled; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Grid", meta = (AllowPrivateAccess = "true"))
	FName RegionId = TEXT("Main");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Grid", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 Columns = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Grid", meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 Rows = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Grid", meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Grid", meta = (AllowPrivateAccess = "true"))
	bool bEnabled = true;
};
