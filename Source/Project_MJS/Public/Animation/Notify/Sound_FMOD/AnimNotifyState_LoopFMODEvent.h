#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_LoopFMODEvent.generated.h"

class UFMODEvent;

UCLASS(meta = (DisplayName = "Loop FMOD Event"))
class PROJECT_MJS_API UAnimNotifyState_LoopFMODEvent : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	TObjectPtr<UFMODEvent> Event = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	FName Handle = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	FName AttachPointName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Volume = 1.0f;

private:
	FName GetEffectiveHandle(USkeletalMeshComponent* MeshComp) const;
};
