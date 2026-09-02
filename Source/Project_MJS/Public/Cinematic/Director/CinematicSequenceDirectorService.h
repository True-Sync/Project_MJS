#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"

class ALevelSequenceActor;
class APlayerController;
class ULevelSequence;
class ULevelSequencePlayer;
class UWorld;

/** Runtime helpers for configuring one spawned Level Sequence actor. */
class PROJECT_MJS_API FCinematicSequenceDirectorService
{
public:
	static ULevelSequencePlayer* CreateSequencePlayer(UWorld* World, ULevelSequence* Sequence, ALevelSequenceActor*& OutSequenceActor);
	static void ConfigureSequenceActor(
		ALevelSequenceActor* SequenceActor,
		APlayerController* PlayerController,
		const FCinematicPlaybackRequest& Request,
		FTransform& OutAnchorWorldTransform,
		bool& bOutAppliedDynamicTransform);
	static void DestroySequenceActor(ALevelSequenceActor* SequenceActor);
};
