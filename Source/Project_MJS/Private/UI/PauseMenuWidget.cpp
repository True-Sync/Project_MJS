#include "UI/PauseMenuWidget.h"

#include "Components/Button.h"

void UPauseMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Back)
	{
		Btn_Back->OnClicked.AddDynamic(this, &UPauseMenuWidget::HandleBackClicked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PauseMenuWidget is missing Btn_Back."));
	}
}

void UPauseMenuWidget::HandleBackClicked()
{
	OnResumeRequested.Broadcast();
}

