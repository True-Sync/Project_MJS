#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "System/Debug/DevCommandDispatcher.h"
#include "DevConsoleSubsystem.generated.h"

UCLASS()
class PROJECT_MJS_API UDevConsoleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 명령 실행 진입점 (UMG 또는 컨트롤러에서 호출)
	UFUNCTION(BlueprintCallable, Category = "DevConsole")
	FString ExecuteCommand(const FString& RawInput);

private:
	void RegisterCoreCommands();

	TArray<FString> RecentLogs;
};
