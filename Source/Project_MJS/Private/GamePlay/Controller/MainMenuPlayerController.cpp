#include "GamePlay/Controller/MainMenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TrueSyncLoadManifest.h"
#include "TrueSyncLoadingSubsystem.h"
#include "Engine/GameInstance.h"

void AMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ConfigureMenuInput(nullptr);
}

void AMainMenuPlayerController::ConfigureMenuInput(UUserWidget* FocusWidget)
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (FocusWidget)
	{
		InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}

	SetInputMode(InputMode);
}

void AMainMenuPlayerController::RequestStartGame(FName LevelName)
{
	if (bIsStartLoading)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 게임 시작 로딩이 진행 중입니다."));
		return;
	}

	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("시작 레벨이 설정되지 않았습니다."));
		return;
	}

	if (!IsValid(CoreLoadManifest))
	{
		UE_LOG(LogTemp, Warning, TEXT("CoreLoadManifest가 설정되지 않았습니다."));
		return;
	}

	UTrueSyncLoadingSubsystem* LoadingSubsystem = GetLoadingSubsystem();
	if (!LoadingSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("로딩 Subsystem을 찾을 수 없습니다."));
		return;
	}

	PendingLevelName = LevelName;
	LoadingText = TEXT("공통 에셋을 준비하는 중입니다.");
	CoreLoadTicket = LoadingSubsystem->StartManifestLoad(CoreLoadManifest, TEXT("MainMenu_CoreAssets"));

	if (!CoreLoadTicket.IsValid())
	{
		HandleStartLoadFailure(TEXT("공통 에셋 로드를 시작하지 못했습니다."));
		return;
	}

	bIsStartLoading = true;

	GetWorldTimerManager().SetTimer(
		CoreLoadPollTimer,
		this,
		&AMainMenuPlayerController::PollCoreLoadState,
		0.05f,
		true);
}

void AMainMenuPlayerController::RequestQuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}


void AMainMenuPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(CoreLoadPollTimer);

	if (bIsStartLoading && CoreLoadTicket.IsValid())
	{
		if (UTrueSyncLoadingSubsystem* LoadingSubsystem = GetLoadingSubsystem())
		{
			LoadingSubsystem->CancelLoad(CoreLoadTicket);
			LoadingSubsystem->ReleaseLoad(CoreLoadTicket);
		}
	}

	Super::EndPlay(EndPlayReason);
}

float AMainMenuPlayerController::GetStartLoadingProgress() const
{
	if (!bIsStartLoading || !CoreLoadTicket.IsValid())
		return 0.0f;

	if (UTrueSyncLoadingSubsystem* LoadingSubsystem = GetLoadingSubsystem())
		return LoadingSubsystem->GetLoadStatus(CoreLoadTicket).Progress;

	return 0.0f;
}

FText AMainMenuPlayerController::GetStartLoadingText() const
{
	return FText::FromString(LoadingText);
}

void AMainMenuPlayerController::PollCoreLoadState()
{
	UTrueSyncLoadingSubsystem* LoadingSubsystem = GetLoadingSubsystem();
	if (!LoadingSubsystem)
	{
		HandleStartLoadFailure(TEXT("로딩 Subsystem이 사라졌습니다."));
		return;
	}

	const FTrueSyncLoadStatus Status = LoadingSubsystem->GetLoadStatus(CoreLoadTicket);

	if (Status.State == ETrueSyncLoadState::LoadingAssets)
	{
		LoadingText = FString::Printf(
			TEXT("공통 에셋을 준비하는 중입니다. %d%%"),
			FMath::RoundToInt(Status.Progress * 100.0f));

		return;
	}

	if (Status.State == ETrueSyncLoadState::Ready)
	{
		GetWorldTimerManager().ClearTimer(CoreLoadPollTimer);

		bIsStartLoading = false;
		LoadingText = TEXT("레벨을 여는 중입니다.");

		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());

		UGameplayStatics::OpenLevel(this, PendingLevelName);
		return;
	}

	HandleStartLoadFailure(Status.ErrorMessage);
}

void AMainMenuPlayerController::HandleStartLoadFailure(const FString& ErrorMessage)
{
	GetWorldTimerManager().ClearTimer(CoreLoadPollTimer);

	if (UTrueSyncLoadingSubsystem* LoadingSubsystem = GetLoadingSubsystem())
		LoadingSubsystem->ReleaseLoad(CoreLoadTicket);

	CoreLoadTicket.Invalidate();
	bIsStartLoading = false;
	LoadingText = ErrorMessage.IsEmpty() ? TEXT("공통 에셋 로드에 실패했습니다.") : ErrorMessage;

	RestoreMenuInput();
}

void AMainMenuPlayerController::RestoreMenuInput()
{
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
}

UTrueSyncLoadingSubsystem* AMainMenuPlayerController::GetLoadingSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UTrueSyncLoadingSubsystem>() : nullptr;
}