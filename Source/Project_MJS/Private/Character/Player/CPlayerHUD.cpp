#include "Character/Player/CPlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void ACPlayerHUD::OnTargetingHUDUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers)
{
	if (UUserWidget* Widget = EnsureCrosshairWidget())
	{
		Widget->SetVisibility(bShowCrosshair ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);

		int32 ViewportSizeX = 0;
		int32 ViewportSizeY = 0;
		if (APlayerController* PlayerController = GetOwningPlayerController())
		{
			PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
		}

		Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));

		const float ViewportScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), KINDA_SMALL_NUMBER);
		Widget->SetPositionInViewport(FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f) / ViewportScale, false);
	}

	for (int32 MarkerIndex = 0; MarkerIndex < Markers.Num(); ++MarkerIndex)
	{
		const FTargetingHUDMarkerData& MarkerData = Markers[MarkerIndex];
		UUserWidget* MarkerWidget = EnsureMarkerWidget(MarkerIndex, GetMarkerWidgetClass(MarkerData.MarkerType));
		if (!MarkerWidget)
		{
			continue;
		}

		MarkerWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
		MarkerWidget->SetPositionInViewport(MarkerData.ScreenPosition, false);
		MarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	HideUnusedMarkerWidgets(Markers.Num());
}

void ACPlayerHUD::OnTargetingHUDCleared()
{
	if (CrosshairWidget)
	{
		CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	HideUnusedMarkerWidgets(0);
}

UUserWidget* ACPlayerHUD::EnsureCrosshairWidget()
{
	APlayerController* OwnerController = GetOwningPlayerController();
	if (!CrosshairWidget && CrosshairWidgetClass && OwnerController)
	{
		CrosshairWidget = CreateWidget<UUserWidget>(OwnerController, CrosshairWidgetClass);
		if (CrosshairWidget)
		{
			CrosshairWidget->AddToViewport(CrosshairZOrder);
			CrosshairWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	return CrosshairWidget;
}

UUserWidget* ACPlayerHUD::EnsureMarkerWidget(int32 MarkerIndex, TSubclassOf<UUserWidget> WidgetClass)
{
	APlayerController* OwnerController = GetOwningPlayerController();
	if (!WidgetClass || !OwnerController)
	{
		return nullptr;
	}

	if (!MarkerWidgets.IsValidIndex(MarkerIndex))
	{
		MarkerWidgets.SetNum(MarkerIndex + 1);
	}

	UUserWidget* ExistingWidget = MarkerWidgets[MarkerIndex];
	if (ExistingWidget && ExistingWidget->GetClass() != WidgetClass)
	{
		ExistingWidget->RemoveFromParent();
		MarkerWidgets[MarkerIndex] = nullptr;
		ExistingWidget = nullptr;
	}

	if (!ExistingWidget)
	{
		ExistingWidget = CreateWidget<UUserWidget>(OwnerController, WidgetClass);
		MarkerWidgets[MarkerIndex] = ExistingWidget;
		if (ExistingWidget)
		{
			ExistingWidget->AddToViewport(MarkerZOrder);
			ExistingWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	return ExistingWidget;
}

TSubclassOf<UUserWidget> ACPlayerHUD::GetMarkerWidgetClass(ETargetingHUDMarkerType MarkerType) const
{
	switch (MarkerType)
	{
	case ETargetingHUDMarkerType::HardTarget:
		return HardTargetWidgetClass;
	case ETargetingHUDMarkerType::AutoTarget:
		return AutoTargetWidgetClass;
	case ETargetingHUDMarkerType::Targetable:
	default:
		return TargetPointWidgetClass;
	}
}

void ACPlayerHUD::HideUnusedMarkerWidgets(int32 FirstUnusedIndex)
{
	for (int32 MarkerIndex = FirstUnusedIndex; MarkerIndex < MarkerWidgets.Num(); ++MarkerIndex)
	{
		if (MarkerWidgets[MarkerIndex])
		{
			MarkerWidgets[MarkerIndex]->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
