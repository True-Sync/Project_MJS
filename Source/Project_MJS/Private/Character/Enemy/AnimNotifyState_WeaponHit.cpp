#include "Character/Enemy/AnimNotifyState_WeaponHit.h"
#include "Character/Enemy/EnemyCharacter.h"

void UAnimNotifyState_WeaponHit::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
		if (Enemy)
		{
			Enemy->StartWeaponTrace();
		}
	}
}

void UAnimNotifyState_WeaponHit::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
		if (Enemy)
		{
			Enemy->StopWeaponTrace();
		}
	}
}