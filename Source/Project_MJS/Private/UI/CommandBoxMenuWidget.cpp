#include "UI/CommandBoxMenuWidget.h"

#include "Components/Button.h"

void UCommandBoxMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Btn_Housing)
	{
		Btn_Housing->OnClicked.AddDynamic(this, &UCommandBoxMenuWidget::HandleHousingClicked);
	}

	if (Btn_Costume)
	{
		Btn_Costume->OnClicked.AddDynamic(this, &UCommandBoxMenuWidget::HandleCostumeClicked);
	}

	if (Btn_StageTravel)
	{
		Btn_StageTravel->OnClicked.AddDynamic(this, &UCommandBoxMenuWidget::HandleStageTravelClicked);
	}

	if (Btn_Close)
	{
		Btn_Close->OnClicked.AddDynamic(this, &UCommandBoxMenuWidget::HandleCloseClicked);
	}
}

void UCommandBoxMenuWidget::HandleHousingClicked()
{
	OnHousingRequested.Broadcast();
}

void UCommandBoxMenuWidget::HandleCostumeClicked()
{
	OnCostumeRequested.Broadcast();
}

void UCommandBoxMenuWidget::HandleStageTravelClicked()
{
	OnStageTravelRequested.Broadcast();
}

void UCommandBoxMenuWidget::HandleCloseClicked()
{
	OnCloseRequested.Broadcast();
}
