#include "Character/Player/CPlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Character/Player/CPlayerController.h"
#include "UI/CommandBoxMenuWidget.h"
#include "UI/GamePlayWidget.h"
#include "UI/HousingMainWidget.h"
#include "UI/PauseMenuWidget.h"
#include "Housing/HousingPlacementComponent.h"

void ACPlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	EnsureGamePlayWidget();
	EnsureHousingMainWidget();
	ValidateGamePlayWidgetConfiguration();
}

void ACPlayerHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ACPlayerHUD::OnTargetingHUDUpdated(bool bShowCrosshair, const TArray<FTargetingHUDMarkerData>& Markers)
{
	if (UGamePlayWidget* Widget = EnsureGamePlayWidget())
	{
		Widget->UpdateTargeting(bShowCrosshair, Markers);
	}
}

void ACPlayerHUD::OnTargetingHUDCleared()
{
	if (GamePlayWidget)
	{
		GamePlayWidget->ClearTargeting();
	}
}

UPauseMenuWidget* ACPlayerHUD::ShowPauseMenu()
{
	UPauseMenuWidget* Widget = EnsurePauseMenuWidget();
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
	}

	return Widget;
}

void ACPlayerHUD::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UCommandBoxMenuWidget* ACPlayerHUD::ShowCommandBoxMenu()
{
	UCommandBoxMenuWidget* Widget = EnsureCommandBoxMenuWidget();
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
	}

	return Widget;
}

void ACPlayerHUD::HideCommandBoxMenu()
{
	if (CommandBoxMenuWidget)
	{
		CommandBoxMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UGamePlayWidget* ACPlayerHUD::EnsureGamePlayWidget()
{
	APlayerController* OwnerController = GetOwningPlayerController();
	if (!GamePlayWidget && GamePlayWidgetClass && OwnerController)
	{
		GamePlayWidget = CreateWidget<UGamePlayWidget>(OwnerController, GamePlayWidgetClass);
		if (GamePlayWidget)
		{
			GamePlayWidget->AddToViewport(GamePlayWidgetZOrder);
			GamePlayWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	return GamePlayWidget;
}

UPauseMenuWidget* ACPlayerHUD::EnsurePauseMenuWidget()
{
	if (PauseMenuWidget)
	{
		return PauseMenuWidget;
	}

	if (!PauseMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD is missing PauseMenuWidgetClass."));
		return nullptr;
	}

	APlayerController* OwnerController = GetOwningPlayerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD could not find an owning player controller for PauseMenuWidget."));
		return nullptr;
	}

	PauseMenuWidget = CreateWidget<UPauseMenuWidget>(OwnerController, PauseMenuWidgetClass);
	if (!PauseMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD failed to create PauseMenuWidget."));
		return nullptr;
	}

	PauseMenuWidget->OnResumeRequested.AddUObject(this, &ACPlayerHUD::HandlePauseMenuResumeRequested);
	PauseMenuWidget->AddToViewport(PauseMenuZOrder);
	PauseMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	return PauseMenuWidget;
}

UCommandBoxMenuWidget* ACPlayerHUD::EnsureCommandBoxMenuWidget()
{
	if (CommandBoxMenuWidget)
	{
		return CommandBoxMenuWidget;
	}

	if (!CommandBoxMenuWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD is missing CommandBoxMenuWidgetClass."));
		return nullptr;
	}

	APlayerController* OwnerController = GetOwningPlayerController();
	if (!OwnerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD could not find an owning player controller for CommandBoxMenuWidget."));
		return nullptr;
	}

	CommandBoxMenuWidget = CreateWidget<UCommandBoxMenuWidget>(OwnerController, CommandBoxMenuWidgetClass);
	if (!CommandBoxMenuWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD failed to create CommandBoxMenuWidget."));
		return nullptr;
	}

	CommandBoxMenuWidget->OnHousingRequested.AddUObject(this, &ACPlayerHUD::HandleCommandBoxHousingRequested);
	CommandBoxMenuWidget->OnCostumeRequested.AddUObject(this, &ACPlayerHUD::HandleCommandBoxCostumeRequested);
	CommandBoxMenuWidget->OnStageTravelRequested.AddUObject(this, &ACPlayerHUD::HandleCommandBoxStageTravelRequested);
	CommandBoxMenuWidget->OnCloseRequested.AddUObject(this, &ACPlayerHUD::HandleCommandBoxCloseRequested);
	CommandBoxMenuWidget->AddToViewport(CommandBoxMenuZOrder);
	CommandBoxMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	return CommandBoxMenuWidget;
}

UHousingMainWidget* ACPlayerHUD::EnsureHousingMainWidget()
{
	if (HousingMainWidget)
	{
		return HousingMainWidget;
	}

	if (!HousingMainWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD is missing HousingMainWidgetClass."));
		return nullptr;
	}

	APlayerController* OwnerController = GetOwningPlayerController();
	if (!OwnerController)
	{
		return nullptr;
	}

	HousingMainWidget = CreateWidget<UHousingMainWidget>(OwnerController, HousingMainWidgetClass);
	if (!HousingMainWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD failed to create HousingMainWidget."));
		return nullptr;
	}

	if (const ACPlayerController* PlayerController = Cast<ACPlayerController>(OwnerController))
	{
		HousingMainWidget->InitializeHousing(PlayerController->GetHousingPlacementComponent());
	}
	HousingMainWidget->AddToViewport(HousingMainWidgetZOrder);
	return HousingMainWidget;
}

void ACPlayerHUD::ValidateGamePlayWidgetConfiguration() const
{
	if (!GamePlayWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD is missing GamePlayWidgetClass."));
		return;
	}

	if (!GetOwningPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD could not find an owning player controller for GamePlayWidget."));
		return;
	}

	if (!GamePlayWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD failed to create GamePlayWidget."));
		return;
	}

	if (!GamePlayWidget->HasTargetingLayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD GamePlayWidget is missing TargetingLayer."));
	}
}

void ACPlayerHUD::HandlePauseMenuResumeRequested()
{
	ACPlayerController* PlayerController = Cast<ACPlayerController>(GetOwningPlayerController());
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD could not find CPlayerController for pause resume."));
		return;
	}

	PlayerController->RequestResumeGame();
}

void ACPlayerHUD::HandleCommandBoxHousingRequested()
{
	if (ACPlayerController* PlayerController = Cast<ACPlayerController>(GetOwningPlayerController()))
	{
		PlayerController->RequestCommandBoxHousing();
	}
}

void ACPlayerHUD::HandleCommandBoxCostumeRequested()
{
	if (ACPlayerController* PlayerController = Cast<ACPlayerController>(GetOwningPlayerController()))
	{
		PlayerController->RequestCommandBoxCostume();
	}
}

void ACPlayerHUD::HandleCommandBoxStageTravelRequested()
{
	if (ACPlayerController* PlayerController = Cast<ACPlayerController>(GetOwningPlayerController()))
	{
		PlayerController->RequestCommandBoxStageTravel();
	}
}

void ACPlayerHUD::HandleCommandBoxCloseRequested()
{
	if (ACPlayerController* PlayerController = Cast<ACPlayerController>(GetOwningPlayerController()))
	{
		PlayerController->CloseCommandBoxMenu();
	}
}
