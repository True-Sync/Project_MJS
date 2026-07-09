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
	
	// ===== 컴포넌트 =====
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TWeakObjectPtr<UCameraComponent> CameraComp;

	// ===== FOV 설정 =====
	// FOV 연출이 끝났을 때 돌아갈 기본 FOV 값
	UPROPERTY(EditAnywhere, Category = "Camera|FOV", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float DefaultFOV = 78.0f;

	// ===== FOV 런타임 상태 =====
	// 현재 FOV 블렌드가 시작된 값
	float FOVStart = 78.0f;

	// 현재 FOV 블렌드가 도달해야 하는 목표 값
	float FOVTarget = 78.0f;

	// FOV 연출 종료 후 복구할 목표 값
	float FOVRestoreTarget = 78.0f;

	// 현재 FOV 단계에서 경과한 시간
	float FOVElapsed = 0.0f;

	// 현재 FOV 단계가 지속되는 시간
	float FOVPhaseDuration = 0.0f;

	// 목표 FOV에 도달한 뒤 유지하는 시간
	float FOVHoldDuration = 0.0f;

	// Hold 이후 기본 FOV로 돌아가는 데 걸리는 시간
	float FOVBlendOutDuration = 0.0f;

	// Hold가 끝난 뒤 FOV를 복구할지 여부
	bool bRestoreFOVAfterHold = true;

	// 현재 FOV 연출 단계
	EFOVPhase FOVPhase = EFOVPhase::None;

	// 현재 FOV 블렌드에 사용하는 보간 타입
	ECameraDirectingFOVBlendType ActiveBlendType = ECameraDirectingFOVBlendType::EaseOut;

	void SetFOVPhase(EFOVPhase NewPhase, float Duration, float NewStartFOV, float NewTargetFOV);
	void ApplyFOVAlpha(float Alpha) const;
	float EvaluateBlend(float Alpha) const;
};
