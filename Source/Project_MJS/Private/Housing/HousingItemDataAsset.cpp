#include "Housing/HousingItemDataAsset.h"

FPrimaryAssetId UHousingItemDataAsset::GetPrimaryAssetId() const
{
	return ItemId.IsNone()
		? Super::GetPrimaryAssetId()
		: FPrimaryAssetId(TEXT("HousingItem"), ItemId);
}
