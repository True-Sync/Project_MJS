#include "UI/HousingItemEntryWidget.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Housing/HousingItemDataAsset.h"

void UHousingItemEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	ItemData = Cast<UHousingItemDataAsset>(ListItemObject);
	if (Img_Thumbnail)
	{
		Img_Thumbnail->SetBrushFromTexture(ItemData ? ItemData->Thumbnail.LoadSynchronous() : nullptr);
	}

	if (Txt_ItemName)
	{
		Txt_ItemName->SetText(ItemData ? ItemData->DisplayName : FText::GetEmpty());
	}

	bSelected = false;
	bHovered = false;
	UpdateVisualState();
}

void UHousingItemEntryWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);

	bSelected = bIsSelected;
	UpdateVisualState();
}

void UHousingItemEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bHovered = true;
	UpdateVisualState();
}

void UHousingItemEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bHovered = false;
	UpdateVisualState();
}

void UHousingItemEntryWidget::UpdateVisualState()
{
	if (Border_Selection)
	{
		Border_Selection->SetBrushColor(bSelected ? SelectedColor : (bHovered ? HoveredColor : UnselectedColor));
	}
}
