#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "CPlayerHUD.generated.h"

class UGamePlayWidget;
class UPauseMenuWidget;
class UUserWidget;

UCLASS()
class PROJECT_MJS_API ACPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	void OnTargetingHUDUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void OnTargetingHUDCleared();

	UPauseMenuWidget* ShowPauseMenu();
	void HidePauseMenu();

private:
	UGamePlayWidget* EnsureGamePlayWidget();
	UPauseMenuWidget* EnsurePauseMenuWidget();
	void ValidateGamePlayWidgetConfiguration() const;

	void HandlePauseMenuResumeRequested();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UGamePlayWidget> GamePlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 GamePlayWidgetZOrder = 0;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 PauseMenuZOrder = 100;

	UPROPERTY(Transient)
	TObjectPtr<UGamePlayWidget> GamePlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;
};
