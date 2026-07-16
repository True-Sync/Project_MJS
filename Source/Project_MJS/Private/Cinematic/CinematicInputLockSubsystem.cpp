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

int32 UCinematicInputLockSubsystem::AcquireInputLock(APlayerController* PlayerController, bool bLockMoveInput, bool bLockLookInput, bool bLockGameplayInput)
{
	if (!PlayerController || (!bLockMoveInput && !bLockLookInput && !bLockGameplayInput))
	{
		return INDEX_NONE;
	}

	const bool bMoveInputAlreadyLocked = IsMoveInputLocked(PlayerController);
	const bool bLookInputAlreadyLocked = IsLookInputLocked(PlayerController);
	const int32 LockHandle = NextLockHandle++;

	FCinematicInputLockRecord& LockRecord = ActiveLocks.Add(LockHandle);
	LockRecord.PlayerController = PlayerController;
	LockRecord.bLockMoveInput = bLockMoveInput;
	LockRecord.bLockLookInput = bLockLookInput;
	LockRecord.bLockGameplayInput = bLockGameplayInput;

	if (bLockMoveInput && !bMoveInputAlreadyLocked)
	{
		PlayerController->SetIgnoreMoveInput(true);
	}

	if (bLockLookInput && !bLookInputAlreadyLocked)
	{
		PlayerController->SetIgnoreLookInput(true);
	}

	return LockHandle;
}

void UCinematicInputLockSubsystem::ReleaseAllInputLocksForPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	TArray<int32> HandlesToRelease;
	for (const TPair<int32, FCinematicInputLockRecord>& LockPair : ActiveLocks)
	{
		if (LockPair.Value.PlayerController.Get() == PlayerController)
		{
			HandlesToRelease.Add(LockPair.Key);
		}
	}

	for (const int32 LockHandle : HandlesToRelease)
	{
		ReleaseInputLock(LockHandle);
	}
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

	// 다른 잠금이 같은 입력을 사용하고 있는지 확인 후 해제
	bool bAnyMoveLocked = false;
	bool bAnyLookLocked = false;

	for (const TPair<int32, FCinematicInputLockRecord>& LockPair : ActiveLocks)
	{
		const FCinematicInputLockRecord& Other = LockPair.Value;
		if (Other.PlayerController.Get() == PlayerController)
		{
			if (Other.bLockMoveInput) bAnyMoveLocked = true;
			if (Other.bLockLookInput) bAnyLookLocked = true;
		}
	}

	if (LockRecord.bLockMoveInput && !bAnyMoveLocked)
	{
		PlayerController->SetIgnoreMoveInput(false);
	}

	if (LockRecord.bLockLookInput && !bAnyLookLocked)
	{
		PlayerController->SetIgnoreLookInput(false);
	}
}

bool UCinematicInputLockSubsystem::IsMoveInputLocked(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	for (const TPair<int32, FCinematicInputLockRecord>& LockPair : ActiveLocks)
	{
		const FCinematicInputLockRecord& LockRecord = LockPair.Value;
		if (LockRecord.PlayerController.Get() == PlayerController && LockRecord.bLockMoveInput)
		{
			return true;
		}
	}

	return false;
}

bool UCinematicInputLockSubsystem::IsLookInputLocked(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	for (const TPair<int32, FCinematicInputLockRecord>& LockPair : ActiveLocks)
	{
		const FCinematicInputLockRecord& LockRecord = LockPair.Value;
		if (LockRecord.PlayerController.Get() == PlayerController && LockRecord.bLockLookInput)
		{
			return true;
		}
	}

	return false;
}

bool UCinematicInputLockSubsystem::IsGameplayInputLocked(const APlayerController* PlayerController) const
{
	if (!PlayerController)
	{
		return false;
	}

	for (const TPair<int32, FCinematicInputLockRecord>& LockPair : ActiveLocks)
	{
		const FCinematicInputLockRecord& LockRecord = LockPair.Value;
		if (LockRecord.PlayerController.Get() == PlayerController && LockRecord.bLockGameplayInput)
		{
			return true;
		}
	}

	return false;
}
