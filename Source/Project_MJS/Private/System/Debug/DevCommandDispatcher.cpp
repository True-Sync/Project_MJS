#include "System/Debug/DevCommandDispatcher.h"

#if !UE_BUILD_SHIPPING

TMap<FString, FDevCommandEntry> FDevCommandDispatcher::Commands;

void FDevCommandDispatcher::RegisterCommand(
	const FString& CommandName,
	const FString& Category,
	const FString& Description,
	TFunction<FString(UWorld* World, const TArray<FString>& Args)> Handler)
{
	FDevCommandEntry Entry;
	Entry.Name = CommandName;
	Entry.Category = Category;
	Entry.Description = Description;
	Entry.Handler = MoveTemp(Handler);

	Commands.Add(CommandName.ToLower(), MoveTemp(Entry));
}

FString FDevCommandDispatcher::ExecuteCommand(UWorld* World, const FString& RawInput)
{
	FString Input = RawInput.TrimStartAndEnd();
	if (Input.IsEmpty())
	{
		return TEXT("Usage: Enter a command. Type 'help' to see available commands.");
	}

	TArray<FString> Tokens;
	Input.ParseIntoArray(Tokens, TEXT(" "), true);
	FString CmdName = Tokens[0].ToLower();

	if (!Commands.Contains(CmdName))
	{
		return FString::Printf(TEXT("Unknown command: %s"), *CmdName);
	}

	TArray<FString> Args;
	for (int32 i = 1; i < Tokens.Num(); ++i)
	{
		Args.Add(Tokens[i]);
	}

	const FDevCommandEntry& Entry = Commands[CmdName];
	return Entry.Handler(World, Args);
}

#endif // !UE_BUILD_SHIPPING
