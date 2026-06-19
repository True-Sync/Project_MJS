#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "Components/ActorComponent.h"
#include "CameraDirectingComponent.generated.h"


class UCameraComponent;

UENUM(BlueprintType)
enum class ECameraDirectingFOVBlendType : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut
};

UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UCameraDirectingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCameraDirectingComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Initialize(UCameraComponent* InCameraComp);

	UFUNCTION(BlueprintCallable, Category = "Camera|Directing")
	void PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Scale = 1.0f, ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal, FRotator UserPlaySpaceRot = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable, Category = "Camera|Directing")
	void PlayFOV(float TargetFOV, float BlendInTime = 0.0f, float HoldTime = 0.0f, float BlendOutTime = 0.0f, bool bRestoreAfterHold = true, bool bUseExplicitStartFOV = false, float ExplicitStartFOV = 0.0f, ECameraDirectingFOVBlendType BlendType = ECameraDirectingFOVBlendType::EaseOut);

	UFUNCTION(BlueprintCallable, Category = "Camera|Directing")
	void EndFOV(float BlendOutTime = 0.0f, ECameraDirectingFOVBlendType BlendType = ECameraDirectingFOVBlendType::EaseOut);

	UFUNCTION(BlueprintCallable, Category = "Camera|Directing")
	void SetFOVImmediately(float NewFOV);

	UFUNCTION(BlueprintCallable, Category = "Camera|Directing")
	void ResetFOV(float BlendTime = 0.0f, ECameraDirectingFOVBlendType BlendType = ECameraDirectingFOVBlendType::EaseOut);
	
private:
	enum class EFOVPhase : uint8
	{
		None,
		BlendIn,
		Hold,
		BlendOut
	};
	
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TWeakObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float DefaultFOV = 78.0f;

	float FOVStart = 78.0f;
	float FOVTarget = 78.0f;
	float FOVRestoreTarget = 78.0f;
	float FOVElapsed = 0.0f;
	float FOVPhaseDuration = 0.0f;
	float FOVHoldDuration = 0.0f;
	float FOVBlendOutDuration = 0.0f;
	bool bRestoreFOVAfterHold = true;
	EFOVPhase FOVPhase = EFOVPhase::None;
	ECameraDirectingFOVBlendType ActiveBlendType = ECameraDirectingFOVBlendType::EaseOut;

	void SetFOVPhase(EFOVPhase NewPhase, float Duration, float NewStartFOV, float NewTargetFOV);
	void ApplyFOVAlpha(float Alpha) const;
	float EvaluateBlend(float Alpha) const;
};
