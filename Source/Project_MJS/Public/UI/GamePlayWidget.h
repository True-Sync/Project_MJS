#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "GamePlayWidget.generated.h"

class UTargetingLayerWidget;
class UPlayerStatusLayerWidget;

UCLASS()
class PROJECT_MJS_API UGamePlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateTargeting(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void ClearTargeting();
	bool HasTargetingLayer() const { return TargetingLayer != nullptr; }
	
	void InitPlayerStatus(class UHealthComponent* PlayerHealthComp);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTargetingLayerWidget> TargetingLayer;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UPlayerStatusLayerWidget> PlayerStatusLayer;
};
