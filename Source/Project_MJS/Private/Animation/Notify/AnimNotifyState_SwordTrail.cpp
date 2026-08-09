#include "Animation/Notify/AnimNotifyState_SwordTrail.h"

#include "Character/Player/Component/AttackComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_SwordTrail::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(
		MeshComp,
		Animation,
		TotalDuration,
		EventReference
	);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;

	UAttackComponent* AttackComponent =
		Owner
		? Owner->FindComponentByClass<UAttackComponent>()
		: nullptr;

	if (AttackComponent)
	{
		AttackComponent->StartSwordTrail();
	}
}

void UAnimNotifyState_SwordTrail::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(
		MeshComp,
		Animation,
		EventReference
	);

	AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr;

	UAttackComponent* AttackComponent =
		Owner
		? Owner->FindComponentByClass<UAttackComponent>()
		: nullptr;

	if (AttackComponent)
	{
		AttackComponent->EndSwordTrail();
	}
}