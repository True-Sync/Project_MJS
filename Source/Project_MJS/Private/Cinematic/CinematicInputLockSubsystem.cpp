#include "Cinematic/CinematicInputLockSubsystem.h"

#include "GameFramework/PlayerController.h"

void UCinematicInputLockSubsystem::Deinitialize()
{
	TArray<int32> LockHandles;
	ActiveLocks.GetKeys(LockHandles);

	for (const int32 LockHandle : LockHandles)
	{
		ReleaseInputLock(LockHandle);
	}

	Super::Deinitialize();
}

int32 UCinematicInputLockSubsystem::AcquireInputLock(APlayerController* PlayerController, bool bLockMoveInput, bool bLockLookInput)
{
	if (!PlayerController || (!bLockMoveInput && !bLockLookInput))
	{
		return INDEX_NONE;
	}

	const int32 LockHandle = NextLockHandle++;

	FCinematicInputLockRecord& LockRecord = ActiveLocks.Add(LockHandle);
	LockRecord.PlayerController = PlayerController;
	LockRecord.bLockMoveInput = bLockMoveInput;
	LockRecord.bLockLookInput = bLockLookInput;

	if (bLockMoveInput)
	{
		PlayerController->SetIgnoreMoveInput(true);
	}

	if (bLockLookInput)
	{
		PlayerController->SetIgnoreLookInput(true);
	}

	return LockHandle;
}

void UCinematicInputLockSubsystem::ReleaseInputLock(int32 LockHandle)
{
	FCinematicInputLockRecord LockRecord;
	if (!ActiveLocks.RemoveAndCopyValue(LockHandle, LockRecord))
	{
		return;
	}

	APlayerController* PlayerController = LockRecord.PlayerController;
	if (!PlayerController)
	{
		return;
	}

	if (LockRecord.bLockMoveInput)
	{
		PlayerController->SetIgnoreMoveInput(false);
	}

	if (LockRecord.bLockLookInput)
	{
		PlayerController->SetIgnoreLookInput(false);
	}
}
