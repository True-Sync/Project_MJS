#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "CPlayerHUD.generated.h"

class UGamePlayWidget;
class UCommandBoxMenuWidget;
class UHousingMainWidget;
class UPauseMenuWidget;
class UUserWidget;

UCLASS()
class PROJECT_MJS_API ACPlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void OnTargetingHUDUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers);
	void OnTargetingHUDCleared();

	UPauseMenuWidget* ShowPauseMenu();
	void HidePauseMenu();

	UCommandBoxMenuWidget* ShowCommandBoxMenu();
	void HideCommandBoxMenu();

private:
	UGamePlayWidget* EnsureGamePlayWidget();
	UPauseMenuWidget* EnsurePauseMenuWidget();
	UCommandBoxMenuWidget* EnsureCommandBoxMenuWidget();
	UHousingMainWidget* EnsureHousingMainWidget();
	void ValidateGamePlayWidgetConfiguration() const;

	void HandlePauseMenuResumeRequested();

	void HandleCommandBoxHousingRequested();

	void HandleCommandBoxCostumeRequested();

	void HandleCommandBoxStageTravelRequested();

	void HandleCommandBoxCloseRequested();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UGamePlayWidget> GamePlayWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UCommandBoxMenuWidget> CommandBoxMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|HUD")
	TSubclassOf<UHousingMainWidget> HousingMainWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 GamePlayWidgetZOrder = 0;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 PauseMenuZOrder = 100;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 CommandBoxMenuZOrder = 50;

	UPROPERTY(EditAnywhere, Category = "Gameplay|HUD")
	int32 HousingMainWidgetZOrder = 60;

	UPROPERTY(Transient)
	TObjectPtr<UGamePlayWidget> GamePlayWidget;

	UPROPERTY(Transient)
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UCommandBoxMenuWidget> CommandBoxMenuWidget;

	UPROPERTY(Transient)
	TObjectPtr<UHousingMainWidget> HousingMainWidget;
};
