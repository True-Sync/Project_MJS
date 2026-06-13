#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayFMODEvent.generated.h"

class UFMODEvent;

UCLASS(meta = (DisplayName = "Play FMOD Event"))
class PROJECT_MJS_API UAnimNotify_PlayFMODEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	TObjectPtr<UFMODEvent> Event = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Volume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD", meta = (ClampMin = "0.0"))
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FMOD")
	FName SocketName = NAME_None;
};
