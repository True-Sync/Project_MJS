#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "HousingItemEntryWidget.generated.h"

class UBorder;
class UHousingItemDataAsset;
class UImage;
class UTextBlock;

UCLASS()
class PROJECT_MJS_API UHousingItemEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual void NativeOnItemSelectionChanged(bool bIsSelected) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	void UpdateVisualState();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_Thumbnail;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_ItemName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> Border_Selection;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Appearance")
	FLinearColor SelectedColor = FLinearColor(0.8f, 0.65f, 0.15f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Appearance")
	FLinearColor HoveredColor = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Appearance")
	FLinearColor UnselectedColor = FLinearColor::White;

	UPROPERTY(Transient)
	TObjectPtr<UHousingItemDataAsset> ItemData;

	bool bSelected = false;
	bool bHovered = false;
};
