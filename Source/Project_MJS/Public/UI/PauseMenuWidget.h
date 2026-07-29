#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

DECLARE_MULTICAST_DELEGATE(FOnPauseMenuResumeRequested);

UCLASS()
class PROJECT_MJS_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnPauseMenuResumeRequested OnResumeRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleBackClicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Back;
};
