#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EnemyFSMComponent.h" // FSM 상태 Enum을 가져오기 위함
#include "AnimNotify_ChangeFSMState.generated.h"

UCLASS()
class PROJECT_MJS_API UAnimNotify_ChangeFSMState : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FSM")
	EEnemyState NextState = EEnemyState::Idle;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};