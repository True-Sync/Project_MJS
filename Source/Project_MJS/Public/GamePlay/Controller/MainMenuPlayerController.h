#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuPlayerController.generated.h"
class UTrueSyncLoadManifest;
class UTrueSyncLoadingSubsystem;

class UUserWidget;

UCLASS()
class PROJECT_MJS_API AMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void ConfigureMenuInput(UUserWidget* FocusWidget);

	void RequestStartGame(FName LevelName);

	void RequestQuitGame();
	
	UFUNCTION(BlueprintPure, Category = "Main Menu|Loading")
	bool IsStartLoading() const { return bIsStartLoading; }

	UFUNCTION(BlueprintPure, Category = "Main Menu|Loading")
	float GetStartLoadingProgress() const;

	UFUNCTION(BlueprintPure, Category = "Main Menu|Loading")
	FText GetStartLoadingText() const;
	
private:
	void PollCoreLoadState();
	void HandleStartLoadFailure(const FString& ErrorMessage);
	void RestoreMenuInput();
	
	UTrueSyncLoadingSubsystem* GetLoadingSubsystem() const;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Main Menu|Loading")
	TObjectPtr<UTrueSyncLoadManifest> CoreLoadManifest = nullptr;
	
private:
	FName PendingLevelName;
	FGuid CoreLoadTicket;
	FTimerHandle CoreLoadPollTimer;

	bool bIsStartLoading = false;
	FString LoadingText;
};
