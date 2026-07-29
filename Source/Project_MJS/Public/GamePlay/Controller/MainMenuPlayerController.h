#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"

UCLASS()
class PROJECT_MJS_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void RequestStartGame(FName LevelName);

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void RequestQuitGame();
};
