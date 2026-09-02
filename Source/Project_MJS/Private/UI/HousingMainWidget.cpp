#include "UI/HousingMainWidget.h"

#include "Housing/HousingPlacementComponent.h"
#include "UI/HousingCatalogWidget.h"
#include "UI/HousingKeyGuideWidget.h"

void UHousingMainWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HousingCatalog)
	{
		HousingCatalog->OnItemSelected.AddUObject(this, &UHousingMainWidget::HandleCatalogItemSelected);
		HousingCatalog->OnCloseRequested.AddUObject(this, &UHousingMainWidget::HandleCatalogCloseRequested);
	}
}

void UHousingMainWidget::NativeDestruct()
{
	if (PlacementComponent)
	{
		PlacementComponent->OnStateChanged.RemoveAll(this);
	}
	if (HousingCatalog)
	{
		HousingCatalog->OnItemSelected.RemoveAll(this);
		HousingCatalog->OnCloseRequested.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UHousingMainWidget::InitializeHousing(UHousingPlacementComponent* InPlacementComponent)
{
	if (PlacementComponent != InPlacementComponent)
	{
		if (PlacementComponent)
		{
			PlacementComponent->OnStateChanged.RemoveAll(this);
		}

		PlacementComponent = InPlacementComponent;
		if (PlacementComponent)
		{
			PlacementComponent->OnStateChanged.AddUObject(this, &UHousingMainWidget::HandleHousingStateChanged);
		}
	}

	if (HousingCatalog)
	{
		HousingCatalog->SetPlacementComponent(PlacementComponent);
	}
	if (HousingKeyGuide)
	{
		HousingKeyGuide->SetGuideContext(PlacementComponent
			? PlacementComponent->GetGuideContext()
			: EHousingGuideContext::Hidden);
	}

	ApplyCurrentState();
}

void UHousingMainWidget::HandleHousingStateChanged(EHousingPlacementState NewState)
{
	ApplyCurrentState();
}

void UHousingMainWidget::HandleCatalogItemSelected(UHousingItemDataAsset* ItemData)
{
	if (PlacementComponent)
	{
		PlacementComponent->BeginPlacement(ItemData);
	}
}

void UHousingMainWidget::HandleCatalogCloseRequested()
{
	if (PlacementComponent && PlacementComponent->GetState() == EHousingPlacementState::CatalogOpen)
	{
		PlacementComponent->ToggleCatalog();
	}
}

void UHousingMainWidget::ApplyCurrentState()
{
	const EHousingPlacementState State = PlacementComponent
		? PlacementComponent->GetState()
		: EHousingPlacementState::Inactive;
	const bool bHousingVisible = State != EHousingPlacementState::Inactive;
	SetVisibility(bHousingVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (HousingCatalog)
	{
		if (State == EHousingPlacementState::CatalogOpen)
		{
			HousingCatalog->RefreshCatalog();
			HousingCatalog->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			HousingCatalog->ResetSelection();
			HousingCatalog->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (HousingKeyGuide)
	{
		HousingKeyGuide->SetGuideContext(PlacementComponent
			? PlacementComponent->GetGuideContext()
			: EHousingGuideContext::Hidden);
	}
}
