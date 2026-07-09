#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Cinematic/CinematicTypes.h"
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
	UFUNCTION(BlueprintPure, Category = "Cinematic|Debug")
	FString GetCinematicStatusSummary() const;

private:
	UFUNCTION()
	void HandleSequenceFinished();

	void FinishCinematic(bool bStopPlayback);
	void RestoreViewTarget();

	bool ShouldAllowPlaybackForNetworkPolicy(const FCinematicPlaybackRequest& Request) const;
	APlayerController* ResolvePlayerController(const FCinematicPlaybackRequest& Request) const;
	void BuildActiveContext(const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform, bool bAppliedDynamicTransform);
	void ApplyBindingOverrides(const FCinematicPlaybackRequest& Request) const;
	void ApplyDynamicTransform(const FCinematicPlaybackRequest& Request, FTransform& OutAnchorWorldTransform, bool& bOutAppliedDynamicTransform) const;
	FTransform ResolveDynamicAnchorTransform(const FCinematicPlaybackRequest& Request) const;
	FTransform ResolveActorOrSocketTransform(const AActor* Actor, FName SocketName) const;
	AActor* ResolveAnchorActor(const FCinematicPlaybackRequest& Request) const;
	FRotator ResolveRotation(const FCinematicPlaybackRequest& Request, const FTransform& AnchorTransform) const;
	FRotator NormalizeCinematicRotation(const FRotator& Rotation, bool bUseYawOnly) const;
	void DrawDebugAnchorTransform(const FCinematicPlaybackRequest& Request, const FTransform& AnchorWorldTransform) const;

	void CollectParticipants(const FCinematicPlaybackRequest& Request);
	void AddActorParticipants(AActor* Actor);
	void AddParticipantObject(UObject* Object);
	void NotifyParticipantsStarted();
	void NotifyParticipantsEnded();

	UPROPERTY(Transient)
	TObjectPtr<ULevelSequencePlayer> ActiveSequencePlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> ActivePlayerController = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PreviousViewTarget = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> ActiveParticipants;

	UPROPERTY(Transient)
	FCinematicPlaybackContext ActiveContext;

	float ActiveBlendOutTime = 0.15f;
	bool bShouldRestoreViewTarget = true;
};
