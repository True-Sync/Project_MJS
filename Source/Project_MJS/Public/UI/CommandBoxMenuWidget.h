#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CommandBoxMenuWidget.generated.h"

class UButton;

DECLARE_MULTICAST_DELEGATE(FCommandBoxMenuRequestSignature);

UCLASS()
class PROJECT_MJS_API UCommandBoxMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FCommandBoxMenuRequestSignature OnHousingRequested;

	FCommandBoxMenuRequestSignature OnCostumeRequested;

	FCommandBoxMenuRequestSignature OnStageTravelRequested;

	FCommandBoxMenuRequestSignature OnCloseRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleHousingClicked();

	UFUNCTION()
	void HandleCostumeClicked();

	UFUNCTION()
	void HandleStageTravelClicked();

	UFUNCTION()
	void HandleCloseClicked();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Housing;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Costume;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_StageTravel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Close;
};
