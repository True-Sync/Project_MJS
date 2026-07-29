#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TrueSyncLoadManifest.generated.h"

UCLASS(BlueprintType)
class TRUESYNCRUNTIMELOADING_API UTrueSyncLoadManifest : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TrueSync|Loading", meta = (AssetBundles = "Preload"))
	TArray<TSoftObjectPtr<UObject>> Assets;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TrueSync|Loading")
	bool bWaitForPSO = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TrueSync|Loading", meta = (ClampMin = "0.0"))
	float MaxPSOWaitSeconds = 30.0f;
};