#include "UI/TargetingLayerWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UTargetingLayerWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ValidateConfiguration();
}

void UTargetingLayerWidget::UpdateTargeting(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers)
{
	if (Crosshair)
	{
		Crosshair->SetVisibility(bShowCrosshair ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	TSet<TWeakObjectPtr<AActor>> ActiveTargets;

	for (const FTargetingHUDMarkerData& MarkerData : Markers)
	{
		AActor* TargetActor = MarkerData.TargetActor.Get();
		if (!TargetActor)
		{
			continue;
		}

		ActiveTargets.Add(TargetActor);
		UUserWidget* MarkerWidget = EnsureMarkerWidget(TargetActor, GetMarkerWidgetClass(MarkerData.MarkerType));
		if (!MarkerWidget)
		{
			continue;
		}

		FVector2D MarkerPosition;
		const bool bOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			GetOwningPlayer(),
			MarkerData.WorldLocation,
			MarkerPosition,
			true);
		if (!bOnScreen)
		{
			MarkerWidget->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MarkerWidget->Slot))
		{
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CanvasSlot->SetPosition(MarkerPosition);
		}
		else
		{
			MarkerWidget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
			MarkerWidget->SetPositionInViewport(MarkerPosition, false);
		}

		MarkerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	ReleaseInactiveMarkerWidgets(ActiveTargets);
}

void UTargetingLayerWidget::ClearTargeting()
{
	if (Crosshair)
	{
		Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}

	const TSet<TWeakObjectPtr<AActor>> EmptyTargets;
	ReleaseInactiveMarkerWidgets(EmptyTargets);
}

UUserWidget* UTargetingLayerWidget::AcquireMarkerWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	if (!MarkerCanvas)
	{
		return nullptr;
	}

	TArray<TObjectPtr<UUserWidget>>& Pool = MarkerWidgetPools.FindOrAdd(WidgetClass.Get());
	while (!Pool.IsEmpty())
	{
		UUserWidget* MarkerWidget = Pool.Pop();
		if (IsValid(MarkerWidget))
		{
			return MarkerWidget;
		}
	}

	UUserWidget* MarkerWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), WidgetClass);
	if (MarkerWidget)
	{
		UCanvasPanelSlot* CanvasSlot = MarkerCanvas->AddChildToCanvas(MarkerWidget);
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MarkerWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	return MarkerWidget;
}

UUserWidget* UTargetingLayerWidget::EnsureMarkerWidget(AActor* TargetActor, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!TargetActor || !WidgetClass)
	{
		return nullptr;
	}

	if (TObjectPtr<UUserWidget>* ExistingWidget = ActiveMarkerWidgets.Find(TargetActor))
	{
		if (IsValid(*ExistingWidget) && (*ExistingWidget)->GetClass() == WidgetClass.Get())
		{
			return ExistingWidget->Get();
		}

		ReleaseMarkerWidget(ExistingWidget->Get());
		ActiveMarkerWidgets.Remove(TargetActor);
	}

	UUserWidget* MarkerWidget = AcquireMarkerWidget(WidgetClass);
	if (MarkerWidget)
	{
		ActiveMarkerWidgets.Add(TargetActor, MarkerWidget);
	}

	return MarkerWidget;
}

void UTargetingLayerWidget::ReleaseMarkerWidget(UUserWidget* MarkerWidget)
{
	if (!IsValid(MarkerWidget))
	{
		return;
	}

	MarkerWidget->SetVisibility(ESlateVisibility::Hidden);
	MarkerWidgetPools.FindOrAdd(MarkerWidget->GetClass()).Add(MarkerWidget);
}

void UTargetingLayerWidget::ReleaseInactiveMarkerWidgets(const TSet<TWeakObjectPtr<AActor>>& ActiveTargets)
{
	for (auto It = ActiveMarkerWidgets.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !ActiveTargets.Contains(It.Key()))
		{
			ReleaseMarkerWidget(It.Value().Get());
			It.RemoveCurrent();
		}
	}
}

void UTargetingLayerWidget::ValidateConfiguration() const
{
	if (!Crosshair)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetingLayerWidget is missing Crosshair widget."));
	}

	if (!MarkerCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetingLayerWidget is missing MarkerCanvas."));
	}

	if (!TargetPointWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetingLayerWidget is missing TargetPointWidgetClass."));
	}

	if (!AutoTargetWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetingLayerWidget is missing AutoTargetWidgetClass."));
	}

	if (!HardTargetWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TargetingLayerWidget is missing HardTargetWidgetClass."));
	}
}

TSubclassOf<UUserWidget> UTargetingLayerWidget::GetMarkerWidgetClass(ETargetingHUDMarkerType MarkerType) const
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
