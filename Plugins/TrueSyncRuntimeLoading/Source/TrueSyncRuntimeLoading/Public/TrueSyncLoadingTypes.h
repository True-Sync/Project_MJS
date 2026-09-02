#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPtr.h"
#include "TrueSyncLoadingTypes.generated.h"

UENUM(BlueprintType)
enum class ETrueSyncLoadState : uint8
{
	Idle,
	LoadingAssets,
	WaitingForPSO,
	Ready,
	Failed,
	Canceled
};

USTRUCT(BlueprintType)
struct TRUESYNCRUNTIMELOADING_API FTrueSyncLoadStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "TrueSync|Loading")
	ETrueSyncLoadState State = ETrueSyncLoadState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "TrueSync|Loading", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Progress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "TrueSync|Loading")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct TRUESYNCRUNTIMELOADING_API FTrueSyncLoadRequest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<UObject>> Assets;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DebugName = TEXT("UnnamedLoad");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWaitForPSO = false;
};
