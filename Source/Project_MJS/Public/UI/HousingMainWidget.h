#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Housing/HousingTypes.h"
#include "HousingMainWidget.generated.h"

class UHousingCatalogWidget;
class UHousingItemDataAsset;
class UHousingKeyGuideWidget;
class UHousingPlacementComponent;

UCLASS()
class PROJECT_MJS_API UHousingMainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeHousing(UHousingPlacementComponent* InPlacementComponent);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void HandleHousingStateChanged(EHousingPlacementState NewState);
	void HandleCatalogItemSelected(UHousingItemDataAsset* ItemData);
	void HandleCatalogCloseRequested();
	void ApplyCurrentState();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHousingCatalogWidget> HousingCatalog;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHousingKeyGuideWidget> HousingKeyGuide;

	UPROPERTY(Transient)
	TObjectPtr<UHousingPlacementComponent> PlacementComponent;
};
