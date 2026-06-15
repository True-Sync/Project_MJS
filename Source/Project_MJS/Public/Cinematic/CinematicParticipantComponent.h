#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Cinematic/CinematicParticipant.h"
#include "CinematicParticipantComponent.generated.h"

class APlayerController;
class UBrainComponent;
class USkeletalMeshComponent;

/*
* UCinematicParticipantComponent
*	시네마틱 참가자 반응 컴포넌트. 
*	플레이어 입력 잠금, 이동 정지, Tick 정지, AI Brain 정지, 애니메이션 일시정지와 복구를 옵션에 따라 처리한다.
*/


UCLASS(ClassGroup=(Cinematic), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UCinematicParticipantComponent : public UActorComponent, public ICinematicParticipant
{
	GENERATED_BODY()

public:
	UCinematicParticipantComponent();

	virtual void OnCinematicStarted_Implementation(const FCinematicPlaybackContext& Context) override;
	virtual void OnCinematicEnded_Implementation(const FCinematicPlaybackContext& Context) override;

	UFUNCTION(BlueprintPure, Category = "Cinematic")
	bool IsInCinematic() const { return ActiveLockCount > 0; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Input")
	bool bLockPlayerMoveInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Input")
	bool bLockPlayerLookInput = true;

	// 공격, 회피, 스킬처럼 Enhanced Input 액션으로 들어오는 전투 입력을 막습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Input")
	bool bLockPlayerGameplayInput = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Movement")
	bool bDisableCharacterMovement = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Tick")
	bool bDisableOwnerTick = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Tick")
	bool bDisableComponentTicks = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|AI")
	bool bPauseAILogic = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Animation")
	bool bPauseSkeletalAnimations = false;

private:
	void ApplyInputLocks(const FCinematicPlaybackContext& Context);
	void ReleaseInputLocks();

	void ApplyMovementLock();
	void ReleaseMovementLock();

	void ApplyTickLocks();
	void ReleaseTickLocks();

	void ApplyAILocks();
	void ReleaseAILocks();
	void TryPauseBrainComponent(UBrainComponent* BrainComponent);

	void ApplyAnimationLocks();
	void ReleaseAnimationLocks();

	APlayerController* ResolvePlayerController(const FCinematicPlaybackContext& Context) const;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> LockedPlayerController = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UActorComponent>> TickLockedComponents;

	UPROPERTY(Transient)
	TArray<bool> SavedComponentTickStates;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UBrainComponent>> PausedBrainComponents;

	UPROPERTY(Transient)
	TArray<TObjectPtr<USkeletalMeshComponent>> PausedSkeletalMeshes;

	UPROPERTY(Transient)
	TArray<bool> SavedMeshPauseStates;

	TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking;
	uint8 SavedCustomMovementMode = 0;
	bool bMoveInputLocked = false;
	bool bLookInputLocked = false;
	bool bMovementModeSaved = false;
	bool bOwnerTickWasEnabled = false;
	int32 InputLockHandle = INDEX_NONE;
	int32 ActiveLockCount = 0;
};
