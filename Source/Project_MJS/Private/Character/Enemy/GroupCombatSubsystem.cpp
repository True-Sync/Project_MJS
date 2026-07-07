#include "Character/Enemy/GroupCombatSubsystem.h"

bool UGroupCombatSubsystem::RequestAttackToken(AActor* Attacker, AActor* Target)
{
	if (!Attacker || !Target) return false;
	
	TArray<AActor*>& Attackers = ActiveAttackersMap.FindOrAdd(Target);
	
	if (Attackers.Contains(Attacker))
	{
		return true;
	}
	
	if (Attackers.Num() < MaxConcurrentAttackers)
	{
		Attackers.Add(Attacker);
		return true;
	}
	
	return false;
}

void UGroupCombatSubsystem::ReleaseAttackToken(AActor* Attacker)
{
	if (!Attacker) return;
	
	for (auto& Pair : ActiveAttackersMap)
	{
		Pair.Value.Remove(Attacker);
	}
}

void UGroupCombatSubsystem::Deinitialize()
{
	ActiveAttackersMap.Empty();
	Super::Deinitialize();
}