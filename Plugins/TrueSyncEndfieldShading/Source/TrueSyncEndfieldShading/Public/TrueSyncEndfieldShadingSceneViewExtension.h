#pragma once

#include "HAL/CriticalSection.h"
#include "SceneViewExtension.h"

class FTrueSyncEndfieldShadingSceneViewExtension final : public FSceneViewExtensionBase
{
public:
	FTrueSyncEndfieldShadingSceneViewExtension(const FAutoRegister& AutoRegister);

	void Invalidate();

	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;

	virtual void SubscribeToPostProcessingPass(
		EPostProcessingPass PassId,
		const FSceneView& View,
		FPostProcessingPassDelegateArray& InOutPassCallbacks,
		bool bIsPassEnabled) override;

protected:
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;

private:
	FScreenPassTexture ApplyEndfieldPass(
		FRDGBuilder& GraphBuilder,
		const FSceneView& View,
		const FPostProcessMaterialInputs& Inputs);

	struct FCachedCameraZoneState
	{
		float InteriorBlend = 0.0f;
		float InteriorHazeMultiplier = 1.0f;
		float InteriorCoolShiftMultiplier = 1.0f;
		float InteriorDesaturationMultiplier = 1.0f;
		float InteriorContrast = 1.0f;
	};

	FCachedCameraZoneState GetCachedCameraZoneState() const;

	TAtomic<bool> bIsAlive;
	mutable FCriticalSection CachedCameraZoneStateMutex;
	FCachedCameraZoneState CachedCameraZoneState;
};
