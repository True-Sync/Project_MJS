#include "Cinematic/Director/CinematicParticipantCoordinator.h"

#include "Cinematic/CinematicParticipant.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void FCinematicParticipantCoordinator::CollectParticipants(UWorld* World, const FCinematicPlaybackRequest& Request)
{
	Reset();

	AddActorParticipants(Request.InstigatorActor);
	AddActorParticipants(Request.SubjectActor);

	for (AActor* ParticipantActor : Request.AdditionalParticipants)
	{
		AddActorParticipants(ParticipantActor);
	}

	if (Request.ParticipantScope != ECinematicParticipantScope::AllInWorld || !World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AddActorParticipants(*It);
	}
}

void FCinematicParticipantCoordinator::NotifyParticipantsStarted(const FCinematicPlaybackContext& Context) const
{
	for (UObject* Participant : Participants)
	{
		if (IsValid(Participant))
		{
			ICinematicParticipant::Execute_OnCinematicStarted(Participant, Context);
		}
	}
}

void FCinematicParticipantCoordinator::NotifyParticipantsEnded(const FCinematicPlaybackContext& Context) const
{
	for (int32 Index = Participants.Num() - 1; Index >= 0; --Index)
	{
		UObject* Participant = Participants[Index];
		if (IsValid(Participant))
		{
			ICinematicParticipant::Execute_OnCinematicEnded(Participant, Context);
		}
	}
}

void FCinematicParticipantCoordinator::Reset()
{
	Participants.Reset();
}

int32 FCinematicParticipantCoordinator::Num() const
{
	return Participants.Num();
}

void FCinematicParticipantCoordinator::AddActorParticipants(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	AddParticipantObject(Actor);

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);
	for (UActorComponent* Component : Components)
	{
		AddParticipantObject(Component);
	}
}

void FCinematicParticipantCoordinator::AddParticipantObject(UObject* Object)
{
	if (!IsValid(Object) || !Object->GetClass()->ImplementsInterface(UCinematicParticipant::StaticClass()))
	{
		return;
	}

	Participants.AddUnique(Object);
}
