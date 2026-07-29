#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

class UUserWidget;

UCLASS()
class PROJECT_MJS_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	void ConfigureMenuInput(UUserWidget* FocusWidget);

	void RequestStartGame(FName LevelName);

	void RequestQuitGame();
};
