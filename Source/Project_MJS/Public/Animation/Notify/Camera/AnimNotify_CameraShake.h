#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Camera/CameraDirectingComponent.h"
#include "Camera/CameraShakeBase.h"
#include "AnimNotify_CameraShake.generated.h"


UCLASS()
class PROJECT_MJS_API UAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Camera|Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditAnywhere, Category = "Camera|Shake", meta = (ClampMin = "0.0"))
	float Scale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Shake")
	ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal;

	UPROPERTY(EditAnywhere, Category = "Camera|Shake")
	FRotator UserPlaySpaceRot = FRotator::ZeroRotator;
};
