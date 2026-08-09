#include "UI/HousingKeyGuideWidget.h"

#include "Components/HorizontalBox.h"

void UHousingKeyGuideWidget::SetGuideContext(EHousingGuideContext NewContext)
{
	CurrentContext = NewContext;
	SetOnlyContextVisible(CurrentContext);
}

void UHousingKeyGuideWidget::SetOnlyContextVisible(EHousingGuideContext Context)
{
	if (HBox_Browsing)
	{
		HBox_Browsing->SetVisibility(Context == EHousingGuideContext::Browsing
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
	if (HBox_CatalogOpen)
	{
		HBox_CatalogOpen->SetVisibility(Context == EHousingGuideContext::CatalogOpen
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
	if (HBox_NewPlacement)
	{
		HBox_NewPlacement->SetVisibility(Context == EHousingGuideContext::NewPlacement
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
	if (HBox_MovingPlacedProp)
	{
		HBox_MovingPlacedProp->SetVisibility(Context == EHousingGuideContext::MovingPlacedProp
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	}
}
