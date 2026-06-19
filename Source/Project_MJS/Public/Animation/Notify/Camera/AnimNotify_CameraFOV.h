#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Camera/CameraDirectingComponent.h"
#include "AnimNotify_CameraFOV.generated.h"


UCLASS()
class PROJECT_MJS_API UAnimNotify_CameraFOV : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UCameraDirectingComponent* FindCameraDirectingComponent(const USkeletalMeshComponent* MeshComp);
	
	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float TargetFOV = 70.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "0.0"))
	float BlendInTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "0.0"))
	float HoldTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "0.0"))
	float BlendOutTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	bool bRestoreAfterHold = true;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	bool bUseExplicitStartFOV = false;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (EditCondition = "bUseExplicitStartFOV", ClampMin = "1.0", ClampMax = "179.0"))
	float ExplicitStartFOV = 78.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV")
	ECameraDirectingFOVBlendType BlendType = ECameraDirectingFOVBlendType::EaseOut;
};
