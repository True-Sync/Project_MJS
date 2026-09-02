#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

class UWorld;

struct FDevCommandEntry
{
	FString Name;
	FString Category;
	FString Description;
	TFunction<FString(UWorld* World, const TArray<FString>& Args)> Handler;
};

class PROJECT_MJS_API FDevCommandDispatcher
{
public:
	static void RegisterCommand(
		const FString& CommandName,
		const FString& Category,
		const FString& Description,
		TFunction<FString(UWorld* World, const TArray<FString>& Args)> Handler);

	static FString ExecuteCommand(UWorld* World, const FString& RawInput);

	static const TMap<FString, FDevCommandEntry>& GetCommands() { return Commands; }

private:
	static TMap<FString, FDevCommandEntry> Commands;
};

#endif // !UE_BUILD_SHIPPING
