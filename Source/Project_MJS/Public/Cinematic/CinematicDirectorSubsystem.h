#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "Cinematic/Director/CinematicParticipantCoordinator.h"
#include "Cinematic/Director/CinematicPostActionExecutor.h"
#include "Subsystems/WorldSubsystem.h"
#include "CinematicDirectorSubsystem.generated.h"

class ALevelSequenceActor;
class ULevelSequencePlayer;

/* 
 * UCinematicDirectorSubsystem
 *	시네마틱 전체 실행 관리자. 
 *	LevelSequencePlayer를 만들고, 
 *	동적 Transform Origin 적용, Binding Override 적용, 
 *	참가자 수집/알림, 종료 후 ViewTarget 복구와 SequenceActor 정리를 담당.
 */

UCLASS()
class PROJECT_MJS_API UCinematicDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool PlayCinematic(const FCinematicPlaybackRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void StopCinematic();

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsCinematicPlaying() const;

	const FCinematicPlaybackContext& GetActiveContext() const { return ActiveContext; }

	// ===== Debug / DevConsole용 상태 요약 =====
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Cinematic|Debug")
	FString GetCinematicStatusSummary() const;

private:
	UFUNCTION()
	void HandleSequenceFinished();

	void FinishCinematic(bool bStopPlayback);
	bool ShouldAllowPlaybackForNetworkPolicy(const FCinematicPlaybackRequest& Request) const;
	APlayerController* ResolvePlayerController(const FCinematicPlaybackRequest& Request) const;
	void BuildActiveContext(const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform, bool bAppliedDynamicTransform);
	void RestoreViewTarget();

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> ActiveSequencePlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> ActivePlayerController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PreviousViewTarget = nullptr;

	UPROPERTY(Transient)
	FCinematicPlaybackContext ActiveContext;

	UPROPERTY(Transient)
	FCinematicParticipantCoordinator ParticipantCoordinator;

	UPROPERTY(Transient)
	FCinematicPostActionExecutor PostActionExecutor;

	float ActiveBlendOutTime = 0.15f;
	bool bShouldRestoreViewTarget = true;
	bool bIsFinishing = false;

	void ExecutePlaybackLevelLoad();
	void ExecutePendingPostAction();
};
