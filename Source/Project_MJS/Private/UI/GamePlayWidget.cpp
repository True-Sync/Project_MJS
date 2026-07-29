#include "UI/GamePlayWidget.h"

#include "UI/TargetingLayerWidget.h"

void UGamePlayWidget::UpdateTargeting(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers)
{
	if (TargetingLayer)
	{
		TargetingLayer->UpdateTargeting(bShowCrosshair, Markers);
	}
}

void UGamePlayWidget::ClearTargeting()
{
	if (TargetingLayer)
	{
		TargetingLayer->ClearTargeting();
	}
}

