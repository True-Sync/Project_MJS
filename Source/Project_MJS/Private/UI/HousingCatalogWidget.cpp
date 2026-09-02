// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HousingCatalogWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/TileView.h"
#include "Engine/Texture2D.h"
#include "Housing/HousingItemDataAsset.h"
#include "Housing/HousingPlacementComponent.h"

void UHousingCatalogWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (TileView_ItemList)
	{
		TileView_ItemList->OnItemClicked().AddUObject(this, &UHousingCatalogWidget::HandleItemClicked);
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleCloseClicked);
	}

	if (Btn_CategoryAll)
	{
		Btn_CategoryAll->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleAllCategoryClicked);
	}
	if (Btn_CategoryFurniture)
	{
		Btn_CategoryFurniture->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleFurnitureCategoryClicked);
	}
	if (Btn_CategoryDecoration)
	{
		Btn_CategoryDecoration->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleDecorationCategoryClicked);
	}
	if (Btn_CategoryLighting)
	{
		Btn_CategoryLighting->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleLightingCategoryClicked);
	}
	if (Btn_CategoryEtc)
	{
		Btn_CategoryEtc->OnClicked.AddDynamic(this, &UHousingCatalogWidget::HandleEtcCategoryClicked);
	}

	RefreshCatalog();
}

void UHousingCatalogWidget::RefreshCatalog()
{
	if (!TileView_ItemList)
	{
		return;
	}

	TArray<UHousingItemDataAsset*> FilteredItems;
	TMap<FName, UHousingItemDataAsset*> ItemsById;
	for (UHousingItemDataAsset* ItemData : CatalogItems)
	{
		if (IsValid(ItemData) && PassesCurrentFilter(ItemData))
		{
			if (!ItemData->ItemId.IsNone())
			{
				if (UHousingItemDataAsset** ExistingItem = ItemsById.Find(ItemData->ItemId))
				{
					UE_LOG(LogTemp, Warning,
						TEXT("Housing catalog has duplicate ItemId '%s': %s and %s. ItemId must be unique."),
						*ItemData->ItemId.ToString(),
						*GetNameSafe(*ExistingItem),
						*GetNameSafe(ItemData));
				}
				else
				{
					ItemsById.Add(ItemData->ItemId, ItemData);
				}
			}
			FilteredItems.Add(ItemData);
		}
	}

	FilteredItems.StableSort([](const UHousingItemDataAsset& Left, const UHousingItemDataAsset& Right)
	{
		return Left.SortOrder < Right.SortOrder;
	});

	TileView_ItemList->ClearListItems();
	for (UHousingItemDataAsset* ItemData : FilteredItems)
	{
		TileView_ItemList->AddItem(ItemData);
	}

	if (SelectedItem && !PassesCurrentFilter(SelectedItem))
	{
		SelectedItem = nullptr;
		UpdateDetails(nullptr);
	}
}

void UHousingCatalogWidget::SetPlacementComponent(UHousingPlacementComponent* InPlacementComponent)
{
	PlacementComponent = InPlacementComponent;
	RefreshCatalog();
}

void UHousingCatalogWidget::SetCategoryFilter(EHousingItemCategory NewCategory)
{
	if (CurrentCategory == NewCategory)
	{
		return;
	}

	CurrentCategory = NewCategory;
	RefreshCatalog();
}

void UHousingCatalogWidget::ResetSelection()
{
	SelectedItem = nullptr;
	if (TileView_ItemList)
	{
		TileView_ItemList->ClearSelection();
	}
	UpdateDetails(nullptr);
}

void UHousingCatalogWidget::HandleItemClicked(UObject* ItemObject)
{
	UHousingItemDataAsset* ItemData = Cast<UHousingItemDataAsset>(ItemObject);
	if (!ItemData)
	{
		return;
	}

	SelectedItem = ItemData;
	UpdateDetails(ItemData);
	OnItemSelected.Broadcast(ItemData);
}

void UHousingCatalogWidget::HandleCloseClicked()
{
	OnCloseRequested.Broadcast();
}

void UHousingCatalogWidget::HandleAllCategoryClicked()
{
	SetCategoryFilter(EHousingItemCategory::All);
}

void UHousingCatalogWidget::HandleFurnitureCategoryClicked()
{
	SetCategoryFilter(EHousingItemCategory::Furniture);
}

void UHousingCatalogWidget::HandleDecorationCategoryClicked()
{
	SetCategoryFilter(EHousingItemCategory::Decoration);
}

void UHousingCatalogWidget::HandleLightingCategoryClicked()
{
	SetCategoryFilter(EHousingItemCategory::Lighting);
}

void UHousingCatalogWidget::HandleEtcCategoryClicked()
{
	SetCategoryFilter(EHousingItemCategory::Etc);
}

void UHousingCatalogWidget::UpdateDetails(UHousingItemDataAsset* ItemData)
{
	if (Txt_ItemName)
	{
		Txt_ItemName->SetText(ItemData ? ItemData->DisplayName : FText::GetEmpty());
	}

	if (Txt_Category)
	{
		const UEnum* CategoryEnum = StaticEnum<EHousingItemCategory>();
		Txt_Category->SetText(ItemData && CategoryEnum
			? CategoryEnum->GetDisplayNameTextByValue(static_cast<int64>(ItemData->Category))
			: FText::GetEmpty());
	}

	if (Txt_Description)
	{
		Txt_Description->SetText(ItemData ? ItemData->Description : FText::GetEmpty());
	}

	if (Img_ItemPreview)
	{
		Img_ItemPreview->SetBrushFromTexture(ItemData ? ItemData->Thumbnail.LoadSynchronous() : nullptr);
	}
}

bool UHousingCatalogWidget::PassesCurrentFilter(const UHousingItemDataAsset* ItemData) const
{
	const bool bMatchesCategory = ItemData
		&& (CurrentCategory == EHousingItemCategory::All || ItemData->Category == CurrentCategory);
	return bMatchesCategory && (!PlacementComponent || PlacementComponent->IsItemAvailable(ItemData));
}

