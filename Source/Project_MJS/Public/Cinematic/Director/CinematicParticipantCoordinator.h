#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "CinematicParticipantCoordinator.generated.h"

class UWorld;
class AActor;

/** Collects cinematic participants and dispatches their lifecycle notifications. */
USTRUCT()
struct PROJECT_MJS_API FCinematicParticipantCoordinator
{
	GENERATED_BODY()

public:
	void CollectParticipants(UWorld* World, const FCinematicPlaybackRequest& Request);
	void NotifyParticipantsStarted(const FCinematicPlaybackContext& Context) const;
	void NotifyParticipantsEnded(const FCinematicPlaybackContext& Context) const;
	void Reset();
	int32 Num() const;

private:
	void AddActorParticipants(AActor* Actor);
	void AddParticipantObject(UObject* Object);

	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Participants;
};
