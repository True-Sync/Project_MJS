#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMainMenuStartRequested, FName);
DECLARE_MULTICAST_DELEGATE(FOnMainMenuSettingRequested);
DECLARE_MULTICAST_DELEGATE(FOnMainMenuExitRequested);

UCLASS()
class PROJECT_MJS_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnMainMenuStartRequested OnStartRequested;

	FOnMainMenuSettingRequested OnSettingRequested;

	FOnMainMenuExitRequested OnExitRequested;

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu")
	FName StartLevelName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Start;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Setting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Exit;

private:
	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleSettingClicked();

	UFUNCTION()
	void HandleExitClicked();
};
