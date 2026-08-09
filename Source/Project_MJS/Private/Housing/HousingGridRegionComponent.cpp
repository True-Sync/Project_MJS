#include "Housing/HousingGridRegionComponent.h"

UHousingGridRegionComponent::UHousingGridRegionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UHousingGridRegionComponent::IsValidCell(int32 X, int32 Y) const
{
	return bEnabled && X >= 0 && X < Columns && Y >= 0 && Y < Rows;
}

bool UHousingGridRegionComponent::WorldToCell(const FVector& WorldLocation, int32& OutX, int32& OutY) const
{
	OutX = INDEX_NONE;
	OutY = INDEX_NONE;
	if (!bEnabled || Columns <= 0 || Rows <= 0 || CellSize <= UE_SMALL_NUMBER)
	{
		return false;
	}

	const FVector LocalLocation = GetComponentTransform().InverseTransformPosition(WorldLocation);
	const float GridWidth = Columns * CellSize;
	const float GridHeight = Rows * CellSize;
	const float LocalX = LocalLocation.X + GridWidth * 0.5f;
	const float LocalY = LocalLocation.Y + GridHeight * 0.5f;

	if (LocalX < 0.0f || LocalX >= GridWidth || LocalY < 0.0f || LocalY >= GridHeight)
	{
		return false;
	}

	OutX = FMath::FloorToInt(LocalX / CellSize);
	OutY = FMath::FloorToInt(LocalY / CellSize);
	return IsValidCell(OutX, OutY);
}

FVector UHousingGridRegionComponent::GetCellCenterWorld(int32 X, int32 Y) const
{
	if (!IsValidCell(X, Y))
	{
		return GetComponentLocation();
	}

	const float GridWidth = Columns * CellSize;
	const float GridHeight = Rows * CellSize;
	const FVector LocalCenter(
		-GridWidth * 0.5f + (X + 0.5f) * CellSize,
		-GridHeight * 0.5f + (Y + 0.5f) * CellSize,
		0.0f);
	return GetComponentTransform().TransformPosition(LocalCenter);
}
