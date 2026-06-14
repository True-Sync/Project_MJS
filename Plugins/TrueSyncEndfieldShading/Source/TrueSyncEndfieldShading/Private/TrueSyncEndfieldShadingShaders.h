#pragma once

#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "SceneRenderTargetParameters.h"
#include "ScreenPass.h"

class FTrueSyncEndfieldCompositePS final : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FTrueSyncEndfieldCompositePS);
	SHADER_USE_PARAMETER_STRUCT(FTrueSyncEndfieldCompositePS, FGlobalShader);

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// ES3_1 이상(iOS Metal 포함)을 지원하는 플랫폼에서 컴파일.
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// 공용
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_STRUCT_INCLUDE(FSceneTextureShaderParameters, SceneTextures)
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, InputViewport)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputSceneColorTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, InputSceneColorSampler)

		// 존 분류
			SHADER_PARAMETER(float, SkyDepthThreshold)
			SHADER_PARAMETER(float, CharacterStencilEnabled)
			SHADER_PARAMETER(float, CharacterStencilValue)
			SHADER_PARAMETER(float, SkinStencilValue)
			SHADER_PARAMETER(float, FaceStencilValue)
			SHADER_PARAMETER(float, HairStencilValue)
			SHADER_PARAMETER(float, ClothStencilValue)
			SHADER_PARAMETER(float, MetalGearStencilValue)
			SHADER_PARAMETER(float, TerrainStencilValue)
			SHADER_PARAMETER(float, IndustrialMetalStencilValue)
			SHADER_PARAMETER(float, ConcreteStencilValue)
			SHADER_PARAMETER(float, VFXStencilValue)
			SHADER_PARAMETER(float, UseMaterialStencilPalette)
			SHADER_PARAMETER(float, CustomDepthCharacterFallback)
			SHADER_PARAMETER(float, CustomDepthFallbackThreshold)

		// Zone B — 캐릭터
		SHADER_PARAMETER(float,     CharacterContrastBoost)
		SHADER_PARAMETER(float,     CharacterSaturationBoost)
			SHADER_PARAMETER(float,     CharacterRimIntensity)
			SHADER_PARAMETER(float,     CharacterRimPower)
			SHADER_PARAMETER(FVector3f, CharacterRimColor)
			SHADER_PARAMETER(float,     CharacterWhiteRimIntensity)
			SHADER_PARAMETER(float,     CharacterWhiteRimPower)
			SHADER_PARAMETER(float,     CharacterWhiteRimHaloIntensity)
			SHADER_PARAMETER(float,     CharacterWhiteRimHaloWidth)
			SHADER_PARAMETER(FVector3f, CharacterWhiteRimColor)
			SHADER_PARAMETER(float,     CharacterOutlineIntensity)
			SHADER_PARAMETER(float,     CharacterOutlineWidth)
			SHADER_PARAMETER(FVector3f, CharacterOutlineColor)

		// Zone C — 환경
		SHADER_PARAMETER(float,     EnvDesaturation)
		SHADER_PARAMETER(float,     EnvCoolShift)
		SHADER_PARAMETER(float,     EnvRimIntensity)
		SHADER_PARAMETER(float,     EnvRimPower)
		SHADER_PARAMETER(FVector3f, EnvRimColor)
		SHADER_PARAMETER(float,     BackgroundWhiteRimIntensity)
		SHADER_PARAMETER(float,     BackgroundWhiteRimPower)
		SHADER_PARAMETER(float,     BackgroundWhiteRimSurfaceStrength)
		SHADER_PARAMETER(float,     BackgroundWhiteRimLumaStart)
		SHADER_PARAMETER(float,     BackgroundWhiteRimLumaEnd)
		SHADER_PARAMETER(float,     BackgroundWhiteRimHighlightContrast)
		SHADER_PARAMETER(FVector3f, BackgroundWhiteRimColor)
		SHADER_PARAMETER(float,     EnvShadowDensity)
		SHADER_PARAMETER(float,     EnvShadowLumaStart)
			SHADER_PARAMETER(float,     EnvShadowLumaEnd)
			SHADER_PARAMETER(float,     EnvShadowHazeSuppression)
			SHADER_PARAMETER(FVector3f, EnvShadowTint)
			SHADER_PARAMETER(float,     ForwardDepthRimIntensity)
			SHADER_PARAMETER(float,     ForwardDepthRimThreshold)

		// Haze
		SHADER_PARAMETER(float,     HazeStartDistance)
		SHADER_PARAMETER(float,     HazeEndDistance)
		SHADER_PARAMETER(float,     HazeIntensity)
		SHADER_PARAMETER(float,     HazeTintStrength)
		SHADER_PARAMETER(FVector3f, HazeTint)

		// Interior Volume
		SHADER_PARAMETER(float, InteriorBlend)
		SHADER_PARAMETER(float, InteriorHazeMultiplier)
		SHADER_PARAMETER(float, InteriorCoolShiftMultiplier)
		SHADER_PARAMETER(float, InteriorDesaturationMultiplier)
		SHADER_PARAMETER(float, InteriorContrast)

		// 디버그
		SHADER_PARAMETER(float, DebugZoneMask)
		SHADER_PARAMETER(float, DebugBackgroundWhiteRim)

		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()
};
