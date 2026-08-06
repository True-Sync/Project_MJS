#include "UI/GamePlayWidget.h"

#include "UI/TargetingLayerWidget.h"
#include "UI/PlayerStatusLayerWidget.h"

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

void UGamePlayWidget::InitPlayerStatus(class UHealthComponent* PlayerHealthComp)
{
	if (PlayerStatusLayer)
	{
		//UE_LOG(LogTemp, Warning, TEXT("UGamePlayWidget::InitPlayerStatus Success"));
		PlayerStatusLayer->InitHealthStatus(PlayerHealthComp);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UGamePlayWidget::InitPlayerStatus Failed: PlayerStatusLayer not exist"));
	}
}

