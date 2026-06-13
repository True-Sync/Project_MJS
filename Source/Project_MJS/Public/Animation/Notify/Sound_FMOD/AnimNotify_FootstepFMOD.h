#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_FootstepFMOD.generated.h"

class UFMODEvent;

UCLASS(meta = (DisplayName = "FMOD Footstep"))
class PROJECT_MJS_API UAnimNotify_FootstepFMOD : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	TObjectPtr<UFMODEvent> EventOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD", meta = (ClampMin = "0.0"))
	float Cooldown = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	FName SocketName = NAME_None;
};
