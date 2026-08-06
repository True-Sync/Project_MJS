#include "Character/Player/CPlayerHUD.h"

#include "Blueprint/UserWidget.h"
#include "Character/Player/CPlayerController.h"
#include "UI/GamePlayWidget.h"
#include "UI/PauseMenuWidget.h"

void ACPlayerHUD::BeginPlay()
{
	Super::BeginPlay();

	EnsureGamePlayWidget();
	ValidateGamePlayWidgetConfiguration();
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

void ACPlayerHUD::InitPlayerHealthUI(class UHealthComponent* HealthComp)
{
	if (UGamePlayWidget* Widget = EnsureGamePlayWidget())
	{
		//UE_LOG(LogTemp, Warning, TEXT("CPlayerHUD::InitPlayerHealthUI Success"));
		Widget->InitPlayerStatus(HealthComp);
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
