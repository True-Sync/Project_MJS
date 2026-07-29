#include "GamePlay/Controller/MainMenuPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}

void AMainMenuPlayerController::RequestStartGame(FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("RequestStartGame failed because LevelName is not set."));
		return;
	}

	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());

	UGameplayStatics::OpenLevel(this, LevelName);
}

void AMainMenuPlayerController::RequestQuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
