#include "UI/MainMenuWidget.h"

#include "Components/Button.h"

void UMainMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Start)
	{
		Btn_Start->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleStartClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget is missing Btn_Start."));
	}

	if (Btn_Setting)
	{
		Btn_Setting->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleSettingClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget is missing Btn_Setting."));
	}

	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UMainMenuWidget::HandleExitClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidget is missing Btn_Exit."));
	}
}

void UMainMenuWidget::HandleStartClicked()
{
	OnStartRequested.Broadcast(StartLevelName);
}

void UMainMenuWidget::HandleSettingClicked()
{
	OnSettingRequested.Broadcast();
}

void UMainMenuWidget::HandleExitClicked()
{
	OnExitRequested.Broadcast();
}
