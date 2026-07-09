#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DevConsoleWidgetBase.generated.h"

class UDevConsoleSubsystem;
class UEditableText;
class UButton;
class UScrollBox;
class UTextBlock;

UCLASS(Abstract, BlueprintType)
class PROJECT_MJS_API UDevConsoleWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UDevConsoleWidgetBase(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	// 초기화 (BP에서 OnInitialized 또는 Construct 후 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "DevConsole")
	void InitializeDevConsole();

	// 명령 실행 진입점 (BP 버튼/Enter 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "DevConsole")
	FString ExecuteCommand(const FString& RawInput);

	// 상태 갱신용 헬퍼 (BP에서 주기적으로 또는 탭 전환 시 호출 권장)
	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetPlayerStatus();

	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetCombatStatus();

	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetCameraTargetingStatus();

	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetCinematicStatus();

	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetAIAndLevelStatus();

	UFUNCTION(BlueprintCallable, Category = "DevConsole|Status")
	FString GetAudioStatus();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableText> WBP_ConsoleInput;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WBP_ConsoleExecuteButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> WBP_LogScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WBP_StatusTextBlock;

	UFUNCTION(BlueprintImplementableEvent, Category = "DevConsole")
	void OnCommandExecuted(const FString& Result);

	UFUNCTION(BlueprintImplementableEvent, Category = "DevConsole")
	void OnStatusUpdated(const FString& StatusText);

	UFUNCTION()
	void OnExecuteButtonClicked();

private:
	TObjectPtr<UDevConsoleSubsystem> DevConsoleSubsystem;

	UObject* GetWidget(); // 내부 헬퍼 (BP 위젯 캐스팅용)
};
