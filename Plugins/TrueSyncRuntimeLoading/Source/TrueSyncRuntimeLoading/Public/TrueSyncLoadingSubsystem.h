#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TrueSyncLoadingTypes.h"
#include "TrueSyncLoadingSubsystem.generated.h"

/**
 * Runtime owner for loading requests. This initial scaffold only exposes the
 * state-query surface; asynchronous requests are added in the next milestone.
 */
UCLASS(BlueprintType)
class TRUESYNCRUNTIMELOADING_API UTrueSyncLoadingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Returns the current state for a loading ticket.
	 *
	 * This scaffold has no request creation API yet, so all tickets report Idle.
	 */
	UFUNCTION(BlueprintPure, Category = "TrueSync|Loading")
	FTrueSyncLoadStatus GetLoadStatus(const FGuid& Ticket) const;
};
