#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrueSyncLoadingTypes.h"
#include "Engine/StreamableManager.h"
#include "TrueSyncLoadingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTrueSyncLoadFinished, FGuid, Ticket, bool, bSuccess,  const FString&, ErrorMessage);

struct FActiveLoad
{
	FTrueSyncLoadRequest Request;
	FTrueSyncLoadStatus Status;
	TSharedPtr<FStreamableHandle> Handle;
	
	bool bCompletionBroadcast = false;

};


class UTrueSyncLoadManifest;

UCLASS(BlueprintType)
class TRUESYNCRUNTIMELOADING_API UTrueSyncLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "TrueSync|Loading")
	FTrueSyncLoadStatus GetLoadStatus(const FGuid& Ticket) const;
	
	UFUNCTION(BlueprintCallable, Category = "TrueSync|Loading")
	FGuid StartLoad(const FTrueSyncLoadRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "TrueSync|Loading")
	FGuid StartManifestLoad(UTrueSyncLoadManifest* Manifest, FString DebugName);
	
	UFUNCTION(BlueprintCallable, Category = "TrueSync|Loading")
	void CancelLoad(FGuid Ticket);

	UFUNCTION(BlueprintCallable, Category = "TrueSync|Loading")
	void ReleaseLoad(FGuid Ticket);
	
	virtual void Deinitialize() override;
	
	
private:
	void HandleAssetsLoaded(FGuid Ticket);
	void BroadcastCompletionOnce(FGuid Ticket, FActiveLoad& Load);
	
	
public:
	UPROPERTY(BlueprintAssignable, Category = "TrueSync|Loading")
	FOnTrueSyncLoadFinished OnLoadFinished;
private:
	TMap<FGuid, FActiveLoad> ActiveLoads;
	
	
};
