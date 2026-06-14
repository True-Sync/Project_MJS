#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CinematicInputLockSubsystem.generated.h"

class APlayerController;

/* 
* UCinematicInputLockSubsystem
*	시네마틱 입력 잠금 전용 서브시스템.
*/

USTRUCT()
struct PROJECT_MJS_API FCinematicInputLockRecord
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(Transient)
	bool bLockMoveInput = false;

	UPROPERTY(Transient)
	bool bLockLookInput = false;
};

UCLASS()
class PROJECT_MJS_API UCinematicInputLockSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	int32 AcquireInputLock(APlayerController* PlayerController, bool bLockMoveInput, bool bLockLookInput);
	void ReleaseInputLock(int32 LockHandle);

private:
	UPROPERTY(Transient)
	TMap<int32, FCinematicInputLockRecord> ActiveLocks;

	int32 NextLockHandle = 1;
};
