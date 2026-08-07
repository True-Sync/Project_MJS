#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Housing/HousingTypes.h"
#include "HousingKeyGuideWidget.generated.h"

class UHorizontalBox;

UCLASS()
class PROJECT_MJS_API UHousingKeyGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetGuideContext(EHousingGuideContext NewContext);

private:
	void SetOnlyContextVisible(EHousingGuideContext Context);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_Browsing;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_CatalogOpen;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_NewPlacement;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HBox_MovingPlacedProp;

	EHousingGuideContext CurrentContext = EHousingGuideContext::Hidden;
};
