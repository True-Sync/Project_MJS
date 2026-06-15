#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Camera/CameraDirectingComponent.h"
#include "AnimNotifyState_CameraFOV.generated.h"

UCLASS()
class PROJECT_MJS_API UAnimNotifyState_CameraFOV : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float TargetFOV = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "0.0"))
	float BlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "0.0"))
	float BlendOutTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	bool bUseExplicitStartFOV = false;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (EditCondition = "bUseExplicitStartFOV", ClampMin = "1.0", ClampMax = "179.0"))
	float ExplicitStartFOV = 78.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	ECameraDirectingFOVBlendType BlendType = ECameraDirectingFOVBlendType::EaseOut;
};
