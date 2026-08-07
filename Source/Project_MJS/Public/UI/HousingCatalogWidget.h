// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Housing/HousingTypes.h"
#include "HousingCatalogWidget.generated.h"

class UButton;
class UHousingItemDataAsset;
class UHousingPlacementComponent;
class UImage;
class UTextBlock;
class UTileView;

DECLARE_MULTICAST_DELEGATE_OneParam(FHousingCatalogItemSelected, UHousingItemDataAsset*);
DECLARE_MULTICAST_DELEGATE(FHousingCatalogCloseRequested);

UCLASS()
class PROJECT_MJS_API UHousingCatalogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FHousingCatalogItemSelected OnItemSelected;
	FHousingCatalogCloseRequested OnCloseRequested;

	UFUNCTION(BlueprintCallable, Category = "Housing|Catalog")
	void RefreshCatalog();

	void SetPlacementComponent(UHousingPlacementComponent* InPlacementComponent);

	UFUNCTION(BlueprintCallable, Category = "Housing|Catalog")
	void SetCategoryFilter(EHousingItemCategory NewCategory);

	UFUNCTION(BlueprintPure, Category = "Housing|Catalog")
	UHousingItemDataAsset* GetSelectedItem() const { return SelectedItem; }

	UFUNCTION(BlueprintCallable, Category = "Housing|Catalog")
	void ResetSelection();

protected:
	virtual void NativeOnInitialized() override;

private:
	void HandleItemClicked(UObject* ItemObject);

	UFUNCTION()
	void HandleCloseClicked();

	UFUNCTION()
	void HandleAllCategoryClicked();

	UFUNCTION()
	void HandleFurnitureCategoryClicked();

	UFUNCTION()
	void HandleDecorationCategoryClicked();

	UFUNCTION()
	void HandleLightingCategoryClicked();

	UFUNCTION()
	void HandleEtcCategoryClicked();

	void UpdateDetails(UHousingItemDataAsset* ItemData);
	bool PassesCurrentFilter(const UHousingItemDataAsset* ItemData) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Housing|Catalog", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UHousingItemDataAsset>> CatalogItems;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTileView> TileView_ItemList;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_ItemName;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Category;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Description;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Img_ItemPreview;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Close;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CategoryAll;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CategoryFurniture;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CategoryDecoration;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CategoryLighting;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CategoryEtc;

	UPROPERTY(Transient)
	TObjectPtr<UHousingItemDataAsset> SelectedItem;

	UPROPERTY(Transient)
	TObjectPtr<UHousingPlacementComponent> PlacementComponent;

	EHousingItemCategory CurrentCategory = EHousingItemCategory::All;
};
