#include "System/Debug/DevConsoleWidgetBase.h"
#include "System/Debug/DevConsoleSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

#if !UE_BUILD_SHIPPING

UDevConsoleWidgetBase::UDevConsoleWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDevConsoleWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (GetWorld())
	{
		DevConsoleSubsystem = GetWorld()->GetSubsystem<UDevConsoleSubsystem>();
	}

	InitializeDevConsole();
}

void UDevConsoleWidgetBase::InitializeDevConsole()
{
	if (!DevConsoleSubsystem)
	{
		return;
	}

	if (UButton* Btn = WBP_ConsoleExecuteButton.Get())
	{
		Btn->OnClicked.RemoveDynamic(this, &UDevConsoleWidgetBase::OnExecuteButtonClicked);
		Btn->OnClicked.AddDynamic(this, &UDevConsoleWidgetBase::OnExecuteButtonClicked);
	}
}

void UDevConsoleWidgetBase::OnExecuteButtonClicked()
{
	UEditableText* InputText = WBP_ConsoleInput.Get();
	if (!InputText)
	{
		OnCommandExecuted(TEXT("Console input widget is not a UEditableText."));
		return;
	}

	ExecuteCommand(InputText->GetText().ToString());
}

FString UDevConsoleWidgetBase::ExecuteCommand(const FString& RawInput)
{
	if (!DevConsoleSubsystem || RawInput.IsEmpty())
	{
		return TEXT("Usage: Enter a command. Type 'help' to see available commands.");
	}

	FString Result = DevConsoleSubsystem->ExecuteCommand(RawInput);

	OnCommandExecuted(Result);
	return Result;
}

FString UDevConsoleWidgetBase::GetPlayerStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	return DevConsoleSubsystem->ExecuteCommand(TEXT("player.status"));
}

FString UDevConsoleWidgetBase::GetCombatStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	return DevConsoleSubsystem->ExecuteCommand(TEXT("combat.status"));
}

FString UDevConsoleWidgetBase::GetCameraTargetingStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	FString Cam = DevConsoleSubsystem->ExecuteCommand(TEXT("camera.status"));
	FString Tgt = DevConsoleSubsystem->ExecuteCommand(TEXT("targeting.status"));
	return FString::Printf(TEXT("%s\n%s"), *Cam, *Tgt);
}

FString UDevConsoleWidgetBase::GetCinematicStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	return DevConsoleSubsystem->ExecuteCommand(TEXT("cinematic.status"));
}

FString UDevConsoleWidgetBase::GetAIAndLevelStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	FString AI = DevConsoleSubsystem->ExecuteCommand(TEXT("ai.status"));
	FString Level = DevConsoleSubsystem->ExecuteCommand(TEXT("level.triggers"));
	return FString::Printf(TEXT("%s\n%s"), *AI, *Level);
}

FString UDevConsoleWidgetBase::GetAudioStatus()
{
	if (!DevConsoleSubsystem) return TEXT("DevConsoleSubsystem not found.");
	return DevConsoleSubsystem->ExecuteCommand(TEXT("audio.status"));
}

UObject* UDevConsoleWidgetBase::GetWidget()
{
	// BP에서 위젯 참조용 헬퍼 (필요 시 사용)
	return this;
}

#endif // !UE_BUILD_SHIPPING
