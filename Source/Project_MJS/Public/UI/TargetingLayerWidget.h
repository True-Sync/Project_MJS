#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "TargetingLayerWidget.generated.h"

class AActor;
class UCanvasPanel;
class UUserWidget;
class UWidget;
class UClass;

UCLASS()
class PROJECT_MJS_API UTargetingLayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTargeting(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void ClearTargeting();

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Crosshair;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> TargetPointWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> AutoTargetWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> HardTargetWidgetClass;

private:
	UUserWidget* AcquireMarkerWidget(TSubclassOf<UUserWidget> WidgetClass);
	UUserWidget* EnsureMarkerWidget(AActor* TargetActor, TSubclassOf<UUserWidget> WidgetClass);
	void ReleaseMarkerWidget(UUserWidget* MarkerWidget);
	void ReleaseInactiveMarkerWidgets(const TSet<TWeakObjectPtr<AActor>>& ActiveTargets);
	void ValidateConfiguration() const;
	TSubclassOf<UUserWidget> GetMarkerWidgetClass(ETargetingHUDMarkerType MarkerType) const;

	TMap<TWeakObjectPtr<AActor>, TObjectPtr<UUserWidget>> ActiveMarkerWidgets;

	TMap<TObjectPtr<UClass>, TArray<TObjectPtr<UUserWidget>>> MarkerWidgetPools;
};
