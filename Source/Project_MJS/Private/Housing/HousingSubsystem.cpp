#include "Housing/HousingSubsystem.h"

#include "Character/Player/CPlayerController.h"
#include "CommandBox/CommandBoxActor.h"
#include "Housing/HousingPlacementComponent.h"

void UHousingSubsystem::HandleCommandBoxHousingRequested(ACommandBoxActor* CommandBox, ACPlayerController* PlayerController)
{
	if (!IsValid(CommandBox))
	{
		return;
	}

	EnterHousing(PlayerController, CommandBox->GetHousingArea());
}

bool UHousingSubsystem::EnterHousing(ACPlayerController* PlayerController, AHousingAreaActor* HousingArea)
{
	UHousingPlacementComponent* PlacementComponent = PlayerController ? PlayerController->GetHousingPlacementComponent() : nullptr;
	return PlacementComponent && PlacementComponent->EnterHousing(HousingArea);
}

void UHousingSubsystem::ExitHousing(ACPlayerController* PlayerController)
{
	if (UHousingPlacementComponent* PlacementComponent = PlayerController ? PlayerController->GetHousingPlacementComponent() : nullptr)
	{
		PlacementComponent->ExitHousing();
	}
}
