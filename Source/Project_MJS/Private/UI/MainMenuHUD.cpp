#include "UI/MainMenuHUD.h"

#include "Blueprint/UserWidget.h"
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
	if (MainMenuWidget)
	{
		MainMenuWidget->AddToViewport();
	}
}
