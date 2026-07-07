#include "Character/Enemy/AnimNotify_ChangeFSMState.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "Character/Enemy/EnemyFSMComponent.h"

void UAnimNotify_ChangeFSMState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(MeshComp->GetOwner());
		if (Enemy && Enemy->FSMComponent)
		{
			Enemy->FSMComponent->ChangeState(NextState);
		}
	}
}