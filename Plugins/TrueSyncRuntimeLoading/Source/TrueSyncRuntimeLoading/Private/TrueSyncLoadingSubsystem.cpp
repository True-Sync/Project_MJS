#include "TrueSyncLoadingSubsystem.h"
#include "Engine/AssetManager.h"
#include "TrueSyncLoadManifest.h"


DEFINE_LOG_CATEGORY_STATIC(LogTrueSyncRuntimeLoading, Log, All);

FTrueSyncLoadStatus UTrueSyncLoadingSubsystem::GetLoadStatus(const FGuid& Ticket) const
{
	if (const FActiveLoad* Load = ActiveLoads.Find(Ticket))
	{
		FTrueSyncLoadStatus Status = Load->Status;
		
		if (Load->Handle.IsValid() && Status.State == ETrueSyncLoadState::LoadingAssets)
		{
			Status.Progress = FMath::Clamp(Load->Handle->GetLoadProgress(), 0.0f, 1.0f);
		}
		
		return Status;
	}
	
	return FTrueSyncLoadStatus();
}


FGuid UTrueSyncLoadingSubsystem::StartLoad(const FTrueSyncLoadRequest& Request)
{
	//실제 오버로드의 경우 에디터에서 재확인 해야함.
	// 넣는 순서는 Handle을 Ticket -> ActiveLoads-> FStreamableHandle -> 로드 완료된 에셋을 메모리에 유지 가능.
	
	/* ====================================================================================================
	(중요)체크해야하는 것. 
		1. IsNull() == false  → 경로가 들어 있음
		2. IsValid() == false → 아직 메모리에 UObject가 없음
		3. IsValid() == true  → 실제 UObject 로드 성공 
		
	Cook에서 빠진 에셋이나 존재하지 않는 경로도 Soft Pointer에는 들어갈 수 있음.. 
	그래서 완료 후 GetLoadedAssets() 결과에 nullptr가 있는지 확인하고 Failed 상태와 오류 로그를 남겨야 함.
	==================================================================================================== */
	
	
	TArray<FSoftObjectPath> Paths;
	for (const TSoftObjectPtr<UObject>& Asset : Request.Assets)
	{
		if (!Asset.IsNull())
			Paths.AddUnique(Asset.ToSoftObjectPath());
	}
	
	if (Paths.IsEmpty())
	{
		UE_LOG(LogTrueSyncRuntimeLoading, Warning, TEXT("로드 요청 실패: 유효한 에셋 경로가 없습니다. DebugName=%s"), *Request.DebugName);

		return FGuid();
	}
	
	
	// Handle 을 ActiveLoads 에 저장해야 로드된 에셋을 필요한 동안 메모리에 유지할 수 있다(Load 변수)
	const FGuid Ticket = FGuid::NewGuid();
	FActiveLoad& Load = ActiveLoads.Add(Ticket);
	Load.Request = Request;
	Load.Status.State = ETrueSyncLoadState::LoadingAssets;
	
	Load.Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		Paths,
		FStreamableDelegate::CreateUObject(this, &UTrueSyncLoadingSubsystem::HandleAssetsLoaded, Ticket),
		FStreamableManager::AsyncLoadHighPriority, false, false, Request.DebugName);
	
	if (!Load.Handle.IsValid())
	{
		UE_LOG(LogTrueSyncRuntimeLoading, Error, TEXT("비동기 로드 요청 생성 실패. DebugName=%s"), *Request.DebugName);
		ActiveLoads.Remove(Ticket);
		return FGuid();
	}

	UE_LOG(LogTrueSyncRuntimeLoading, Log, TEXT("비동기 로드 시작. Ticket=%s DebugName=%s AssetCount=%d"), *Ticket.ToString(), *Request.DebugName, Paths.Num());
	
	return Ticket;
}

FGuid UTrueSyncLoadingSubsystem::StartManifestLoad(UTrueSyncLoadManifest* Manifest, FString DebugName)
{
	if (!IsValid(Manifest))
	{
		UE_LOG(LogTrueSyncRuntimeLoading, Warning, TEXT("매니페스트 로드 요청 실패: Manifest가 유효하지 않습니다."));
		return FGuid();
	}
	
	FTrueSyncLoadRequest Request;
	Request.Assets = Manifest->Assets;
	Request.DebugName = DebugName.IsEmpty() ? Manifest->GetName() : MoveTemp(DebugName);
	
	Request.bWaitForPSO = Manifest->bWaitForPSO;
	
	return StartLoad(Request);
}

void UTrueSyncLoadingSubsystem::HandleAssetsLoaded(FGuid Ticket)
{
	FActiveLoad* Load = ActiveLoads.Find(Ticket);
	
	if (!Load || Load->Status.State != ETrueSyncLoadState::LoadingAssets)
		return;
	
	TArray<UObject*> LoadedAssets;
	Load->Handle->GetLoadedAssets(LoadedAssets);
	
	const bool bHasFailure = LoadedAssets.ContainsByPredicate(
		[](const UObject* Asset)
		{
			return !IsValid(Asset);
		});
	
	Load->Status.State = bHasFailure ? ETrueSyncLoadState::Failed : ETrueSyncLoadState::Ready;
	Load->Status.Progress = 1.0f;
	
	if (bHasFailure)
	{
		Load->Status.ErrorMessage = TEXT("하나 이상의 에셋을 로드하지 못했습니다.");
		UE_LOG( LogTrueSyncRuntimeLoading, Error, TEXT("비동기 로드 실패. Ticket=%s DebugName=%s"), *Ticket.ToString(), *Load->Request.DebugName);
	}
	else
		UE_LOG( LogTrueSyncRuntimeLoading, Log, TEXT("비동기 로드 완료. Ticket=%s DebugName=%s"), *Ticket.ToString(), *Load->Request.DebugName);
	
	BroadcastCompletionOnce(Ticket, *Load);

}

void UTrueSyncLoadingSubsystem::CancelLoad(FGuid Ticket)
{
	FActiveLoad* Load = ActiveLoads.Find(Ticket);
	if (!Load || Load->Status.State != ETrueSyncLoadState::LoadingAssets)
		return;
	
	if (Load->Handle.IsValid())
		Load->Handle->CancelHandle();

	Load->Status.State = ETrueSyncLoadState::Canceled;
	Load->Status.ErrorMessage = TEXT("사용자 요청으로 로드를 취소했습니다.");
	
	BroadcastCompletionOnce(Ticket, *Load);
}

void UTrueSyncLoadingSubsystem::ReleaseLoad(FGuid Ticket)
{
	FActiveLoad* Load = ActiveLoads.Find(Ticket);
	if (!Load)
		return;
	if (Load->Status.State == ETrueSyncLoadState::LoadingAssets)
	{
		CancelLoad(Ticket);
		Load = ActiveLoads.Find(Ticket);
	}
	
	if (Load && Load->Handle.IsValid())
		Load->Handle->ReleaseHandle();
	
	ActiveLoads.Remove(Ticket);
}

void UTrueSyncLoadingSubsystem::Deinitialize()
{
	for (TPair<FGuid, FActiveLoad>& Pair : ActiveLoads)
	{
		if (Pair.Value.Handle.IsValid())
		{
			Pair.Value.Handle->CancelHandle();
			Pair.Value.Handle->ReleaseHandle();
		}
	}
	
	ActiveLoads.Empty();
	Super::Deinitialize();
}

void UTrueSyncLoadingSubsystem::BroadcastCompletionOnce(FGuid Ticket, FActiveLoad& Load)
{
	if (Load.bCompletionBroadcast)
		return;
	
	Load.bCompletionBroadcast = true;
	
	const bool bSuccess = Load.Status.State == ETrueSyncLoadState::Ready;
	OnLoadFinished.Broadcast(Ticket, bSuccess, Load.Status.ErrorMessage);
}
