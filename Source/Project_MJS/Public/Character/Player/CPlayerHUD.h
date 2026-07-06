#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "CPlayerHUD.generated.h"

class UUserWidget;

UCLASS()
class PROJECT_MJS_API ACPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	void OnTargetingHUDUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void OnTargetingHUDCleared();

private:
	UUserWidget* EnsureCrosshairWidget();
	UUserWidget* EnsureMarkerWidget(int32 MarkerIndex, TSubclassOf<UUserWidget> WidgetClass);
	TSubclassOf<UUserWidget> GetMarkerWidgetClass(ETargetingHUDMarkerType MarkerType) const;
	void HideUnusedMarkerWidgets(int32 FirstUnusedIndex);

	// ===== 타겟팅 UI 위젯 =====
	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> TargetPointWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> AutoTargetWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting|HUD")
	TSubclassOf<UUserWidget> HardTargetWidgetClass;

	// 크로스헤어가 Viewport에 올라갈 때 사용할 ZOrder
	UPROPERTY(EditAnywhere, Category = "Targeting|HUD")
	int32 CrosshairZOrder = 10;

	// 타겟 마커들이 Viewport에 올라갈 때 사용할 ZOrder
	UPROPERTY(EditAnywhere, Category = "Targeting|HUD")
	int32 MarkerZOrder = 11;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UUserWidget>> MarkerWidgets;
};
