#include "UI/MainMenuWidget.h"

#include "Components/Button.h"
#include "GamePlay/Controller/MainMenuPlayerController.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStartClicked);
	}

	if (Btn_Setting)
	{
		Btn_Setting->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettingClicked);
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleExitClicked);
	}
}

void UMainMenuWidget::HandleStartClicked()
{
	AMainMenuPlayerController* MainMenuPlayerController = Cast<AMainMenuPlayerController>(GetOwningPlayer());
	if (!MainMenuPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget could not find MainMenuPlayerController."));
		return;
	}

	MainMenuPlayerController->RequestStartGame(StartLevelName);
}

void UMainMenuWidget::HandleSettingClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Setting button clicked. Settings widget is not implemented yet."));
}

void UMainMenuWidget::HandleExitClicked()
{
	AMainMenuPlayerController* MainMenuPlayerController = Cast<AMainMenuPlayerController>(GetOwningPlayer());
	if (!MainMenuPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget could not find MainMenuPlayerController."));
		return;
	}

	MainMenuPlayerController->RequestQuitGame();
}
