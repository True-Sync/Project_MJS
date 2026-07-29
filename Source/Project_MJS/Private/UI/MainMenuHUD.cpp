#include "UI/MainMenuHUD.h"

#include "Blueprint/UserWidget.h"
#include "GamePlay/Controller/MainMenuPlayerController.h"
#include "UI/MainMenuWidget.h"

void AMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	if (!MainMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuWidgetClass is not set on %s."), *GetName());
		return;
	}

	APlayerController* OwningPlayerController = GetOwningPlayerController();
	if (!OwningPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuHUD could not find an owning player controller."));
		return;
	}

	MainMenuWidget = CreateWidget<UMainMenuWidget>(OwningPlayerController, MainMenuWidgetClass);
	if (!MainMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuHUD failed to create MainMenuWidget."));
		return;
	}

	MainMenuWidget->OnStartRequested.AddUObject(this, &AMainMenuHUD::HandleStartRequested);
	MainMenuWidget->OnSettingRequested.AddUObject(this, &AMainMenuHUD::HandleSettingRequested);
	MainMenuWidget->OnExitRequested.AddUObject(this, &AMainMenuHUD::HandleExitRequested);
	MainMenuWidget->AddToViewport();

	if (AMainMenuPlayerController* MainMenuPlayerController = Cast<AMainMenuPlayerController>(OwningPlayerController))
	{
		MainMenuPlayerController->ConfigureMenuInput(MainMenuWidget);
	}
}

void AMainMenuHUD::HandleStartRequested(FName LevelName)
{
	AMainMenuPlayerController* MainMenuPlayerController = Cast<AMainMenuPlayerController>(GetOwningPlayerController());
	if (!MainMenuPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuHUD could not find MainMenuPlayerController."));
		return;
	}

	MainMenuPlayerController->RequestStartGame(LevelName);
}

void AMainMenuHUD::HandleSettingRequested()
{
	UE_LOG(LogTemp, Log, TEXT("Setting button clicked. Settings widget is not implemented yet."));
}

void AMainMenuHUD::HandleExitRequested()
{
	AMainMenuPlayerController* MainMenuPlayerController = Cast<AMainMenuPlayerController>(GetOwningPlayerController());
	if (!MainMenuPlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenuHUD could not find MainMenuPlayerController."));
		return;
	}

	MainMenuPlayerController->RequestQuitGame();
}
