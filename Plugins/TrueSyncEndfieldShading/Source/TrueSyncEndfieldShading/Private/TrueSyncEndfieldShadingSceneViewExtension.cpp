#include "TrueSyncEndfieldShadingSceneViewExtension.h"

#include "TrueSyncEndfieldInteriorVolume.h"
#include "TrueSyncEndfieldShadingModule.h"
#include "TrueSyncEndfieldShadingShaders.h"
#include "TrueSyncEndfieldShadingWorldSubsystem.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/ScopeLock.h"
#include "PostProcess/PostProcessMaterialInputs.h"
#include "RenderGraphBuilder.h"
#include "RHIStaticStates.h"
#include "ScreenPass.h"

namespace TrueSyncEndfieldShading
{
	static UWorld* ResolveViewWorld(const FSceneView& InView)
	{
		if (const AActor* ViewActor = InView.ViewActor)
		{
			return ViewActor->GetWorld();
		}

		if (GEngine)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				if (!World) { continue; }
				if (WorldContext.WorldType == EWorldType::PIE
					|| WorldContext.WorldType == EWorldType::Game
					|| WorldContext.WorldType == EWorldType::Editor)
				{
					return World;
				}
			}
		}
		return nullptr;
	}

	// -----------------------------------------------------------------------
	// 시스템
	// -----------------------------------------------------------------------
	static TAutoConsoleVariable<int32> CVarEnable(
		TEXT("r.TrueSyncEndfieldShading.Enable"), 1,
		TEXT("Enable the Endfield-style post process pass.\n0: Disabled\n1: Enabled"),
		ECVF_RenderThreadSafe);

	// -----------------------------------------------------------------------
	// 존 분류
	// -----------------------------------------------------------------------
	static TAutoConsoleVariable<float> CVarSkyDepthThreshold(
		TEXT("r.TrueSyncEndfieldShading.SkyDepthThreshold"), 150000.0f,
		TEXT("LinearDepth at which pixels are classified as Sky (fully unprocessed).\n"
		     "Sky zone transitions smoothly over the last 5%% of this distance."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarCharacterStencilEnable(
		TEXT("r.TrueSyncEndfieldShading.CharacterStencilEnable"), 1,
		TEXT("Enable Character CustomStencil zone separation.\n0: Disabled\n1: Enabled"),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarCharacterStencilValue(
		TEXT("r.TrueSyncEndfieldShading.CharacterStencilValue"), 1,
		TEXT("CustomStencil value used to identify character pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarSkinStencilValue(
		TEXT("r.TrueSyncEndfieldShading.SkinStencilValue"), 11,
		TEXT("CustomStencil value used to identify skin pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarFaceStencilValue(
		TEXT("r.TrueSyncEndfieldShading.FaceStencilValue"), 12,
		TEXT("CustomStencil value used to identify face pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarHairStencilValue(
		TEXT("r.TrueSyncEndfieldShading.HairStencilValue"), 13,
		TEXT("CustomStencil value used to identify hair pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarClothStencilValue(
		TEXT("r.TrueSyncEndfieldShading.ClothStencilValue"), 14,
		TEXT("CustomStencil value used to identify cloth pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarMetalGearStencilValue(
		TEXT("r.TrueSyncEndfieldShading.MetalGearStencilValue"), 15,
		TEXT("CustomStencil value used to identify character metal/gear pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarTerrainStencilValue(
		TEXT("r.TrueSyncEndfieldShading.TerrainStencilValue"), 31,
		TEXT("CustomStencil value used to identify terrain pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarIndustrialMetalStencilValue(
		TEXT("r.TrueSyncEndfieldShading.IndustrialMetalStencilValue"), 32,
		TEXT("CustomStencil value used to identify industrial metal environment pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarConcreteStencilValue(
		TEXT("r.TrueSyncEndfieldShading.ConcreteStencilValue"), 33,
		TEXT("CustomStencil value used to identify concrete environment pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarVFXStencilValue(
		TEXT("r.TrueSyncEndfieldShading.VFXStencilValue"), 41,
		TEXT("CustomStencil value used to identify VFX pixels that should avoid heavy environment grading."),
		ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<int32> CVarUseMaterialStencilPalette(
			TEXT("r.TrueSyncEndfieldShading.UseMaterialStencilPalette"), 1,
			TEXT("Enable fine material-zone classification from CustomStencil palette."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<int32> CVarCustomDepthCharacterFallback(
			TEXT("r.TrueSyncEndfieldShading.CustomDepthCharacterFallback"), 1,
			TEXT("Classify pixels with matching CustomDepth as character when CustomStencil is unavailable or zero."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCustomDepthFallbackThreshold(
			TEXT("r.TrueSyncEndfieldShading.CustomDepthFallbackThreshold"), 8.0f,
			TEXT("Allowed depth difference in cm for CustomDepth character fallback."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarKeyLightDirX(
			TEXT("r.TrueSyncEndfieldShading.KeyLightDirX"), -0.35f,
			TEXT("World-space key light direction X used by post cel bands and directional character rim."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarKeyLightDirY(
			TEXT("r.TrueSyncEndfieldShading.KeyLightDirY"), -0.45f,
			TEXT("World-space key light direction Y used by post cel bands and directional character rim."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarKeyLightDirZ(
			TEXT("r.TrueSyncEndfieldShading.KeyLightDirZ"), 0.82f,
			TEXT("World-space key light direction Z used by post cel bands and directional character rim."),
			ECVF_RenderThreadSafe);

		static FVector3f GetKeyLightDirectionOnRenderThread()
		{
			const FVector3f Direction(
				CVarKeyLightDirX.GetValueOnRenderThread(),
				CVarKeyLightDirY.GetValueOnRenderThread(),
				CVarKeyLightDirZ.GetValueOnRenderThread());
			const float LengthSquared = Direction.SizeSquared();
			if (LengthSquared <= UE_SMALL_NUMBER)
			{
				return FVector3f(-0.35f, -0.45f, 0.82f).GetSafeNormal();
			}
			return Direction * FMath::InvSqrt(LengthSquared);
		}

		// -----------------------------------------------------------------------
		// Zone B — 캐릭터
		// -----------------------------------------------------------------------
	static TAutoConsoleVariable<float> CVarCharacterContrastBoost(
		TEXT("r.TrueSyncEndfieldShading.CharacterContrastBoost"), 1.03f,
		TEXT("Contrast multiplier applied to character pixels. >1 boosts pop."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterSaturationBoost(
		TEXT("r.TrueSyncEndfieldShading.CharacterSaturationBoost"), 1.02f,
		TEXT("Saturation multiplier for character pixels. Preserves vibrant character colors."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadingStrength(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadingStrength"), 0.34f,
		TEXT("Strength of key-light cel shadow bands on character pixels. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowThreshold(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowThreshold"), 0.44f,
		TEXT("NdotL threshold where character cel shadows start."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowSoftness(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowSoftness"), 0.10f,
		TEXT("Softness around the character cel shadow threshold."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowDepth(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowDepth"), 0.24f,
		TEXT("How dark the character cel shadow band can become."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowR(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowR"), 0.72f,
		TEXT("Character cel shadow tint red channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowG(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowG"), 0.76f,
		TEXT("Character cel shadow tint green channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterCelShadowB(
		TEXT("r.TrueSyncEndfieldShading.CharacterCelShadowB"), 0.88f,
		TEXT("Character cel shadow tint blue channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterRimIntensity(
		TEXT("r.TrueSyncEndfieldShading.CharacterRimIntensity"), 0.08f,
		TEXT("Rim light intensity on character pixels. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterRimPower(
		TEXT("r.TrueSyncEndfieldShading.CharacterRimPower"), 4.0f,
		TEXT("Rim light falloff exponent on character pixels. Higher = tighter rim."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterDirectionalRimStrength(
		TEXT("r.TrueSyncEndfieldShading.CharacterDirectionalRimStrength"), 1.0f,
		TEXT("Blend character rim from camera-only silhouette to key-light-side silhouette. 0=old full rim, 1=directional."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterDirectionalRimLightStart(
		TEXT("r.TrueSyncEndfieldShading.CharacterDirectionalRimLightStart"), 0.05f,
		TEXT("Wrapped NdotL value where directional character rim starts."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterDirectionalRimLightEnd(
		TEXT("r.TrueSyncEndfieldShading.CharacterDirectionalRimLightEnd"), 0.48f,
		TEXT("Wrapped NdotL value where directional character rim reaches full strength."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterDirectionalRimLightWrap(
		TEXT("r.TrueSyncEndfieldShading.CharacterDirectionalRimLightWrap"), 0.10f,
		TEXT("Wrap added to NdotL for directional character rim. Higher values let rim bleed farther around forms."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarCharacterRimR(
		TEXT("r.TrueSyncEndfieldShading.CharacterRimR"), 0.80f,
		TEXT("Character rim light color — Red channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarCharacterRimG(
		TEXT("r.TrueSyncEndfieldShading.CharacterRimG"), 0.90f,
		TEXT("Character rim light color — Green channel."), ECVF_RenderThreadSafe);
		static TAutoConsoleVariable<float> CVarCharacterRimB(
			TEXT("r.TrueSyncEndfieldShading.CharacterRimB"), 1.00f,
			TEXT("Character rim light color — Blue channel."), ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterWhiteRimIntensity(
			TEXT("r.TrueSyncEndfieldShading.CharacterWhiteRimIntensity"), 0.15f,
			TEXT("HDR white rim intensity on character pixels. Use this for strong anime backlight rims."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterWhiteRimPower(
			TEXT("r.TrueSyncEndfieldShading.CharacterWhiteRimPower"), 2.2f,
			TEXT("White rim falloff exponent. Lower values make the rim wider."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterWhiteRimR(
			TEXT("r.TrueSyncEndfieldShading.CharacterWhiteRimR"), 1.00f,
			TEXT("Character white rim color — Red channel."), ECVF_RenderThreadSafe);
		static TAutoConsoleVariable<float> CVarCharacterWhiteRimG(
			TEXT("r.TrueSyncEndfieldShading.CharacterWhiteRimG"), 0.97f,
			TEXT("Character white rim color — Green channel."), ECVF_RenderThreadSafe);
		static TAutoConsoleVariable<float> CVarCharacterWhiteRimB(
			TEXT("r.TrueSyncEndfieldShading.CharacterWhiteRimB"), 0.90f,
			TEXT("Character white rim color — Blue channel."), ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterOutlineIntensity(
			TEXT("r.TrueSyncEndfieldShading.CharacterOutlineIntensity"), 0.35f,
			TEXT("Dark screen-space outline intensity outside character silhouettes."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterOutlineWidth(
			TEXT("r.TrueSyncEndfieldShading.CharacterOutlineWidth"), 1.25f,
			TEXT("Screen-space character outline sample width in pixels."),
			ECVF_RenderThreadSafe);

		static TAutoConsoleVariable<float> CVarCharacterOutlineR(
			TEXT("r.TrueSyncEndfieldShading.CharacterOutlineR"), 0.025f,
			TEXT("Character outline color — Red channel."), ECVF_RenderThreadSafe);
		static TAutoConsoleVariable<float> CVarCharacterOutlineG(
			TEXT("r.TrueSyncEndfieldShading.CharacterOutlineG"), 0.030f,
			TEXT("Character outline color — Green channel."), ECVF_RenderThreadSafe);
		static TAutoConsoleVariable<float> CVarCharacterOutlineB(
			TEXT("r.TrueSyncEndfieldShading.CharacterOutlineB"), 0.040f,
			TEXT("Character outline color — Blue channel."), ECVF_RenderThreadSafe);

		// -----------------------------------------------------------------------
		// Zone C — 환경/배경
		// -----------------------------------------------------------------------
	static TAutoConsoleVariable<float> CVarEnvDesaturation(
		TEXT("r.TrueSyncEndfieldShading.EnvDesaturation"), 0.06f,
		TEXT("Desaturation amount for environment pixels. Makes environment recede behind characters."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCoolShift(
		TEXT("r.TrueSyncEndfieldShading.EnvCoolShift"), 0.05f,
		TEXT("Cool (cyan-blue) color shift applied to environment. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadingStrength(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadingStrength"), 0.18f,
		TEXT("Strength of key-light cel shadow bands on environment pixels. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowThreshold(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowThreshold"), 0.40f,
		TEXT("NdotL threshold where environment cel shadows start."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowSoftness(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowSoftness"), 0.18f,
		TEXT("Softness around the environment cel shadow threshold."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowDepth(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowDepth"), 0.18f,
		TEXT("How dark the environment cel shadow band can become."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowR(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowR"), 0.68f,
		TEXT("Environment cel shadow tint red channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowG(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowG"), 0.74f,
		TEXT("Environment cel shadow tint green channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelShadowB(
		TEXT("r.TrueSyncEndfieldShading.EnvCelShadowB"), 0.84f,
		TEXT("Environment cel shadow tint blue channel."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelFadeStartDistance(
		TEXT("r.TrueSyncEndfieldShading.EnvCelFadeStartDistance"), 4500.0f,
		TEXT("Distance where environment cel bands start fading out to avoid far-grid shimmer/stripes."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvCelFadeEndDistance(
		TEXT("r.TrueSyncEndfieldShading.EnvCelFadeEndDistance"), 13000.0f,
		TEXT("Distance where environment cel bands are fully faded out."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvRimIntensity(
		TEXT("r.TrueSyncEndfieldShading.EnvRimIntensity"), 0.015f,
		TEXT("Rim light intensity on environment pixels. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvRimPower(
		TEXT("r.TrueSyncEndfieldShading.EnvRimPower"), 3.0f,
		TEXT("Rim light falloff exponent on environment pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvRimR(
		TEXT("r.TrueSyncEndfieldShading.EnvRimR"), 0.70f,
		TEXT("Environment rim light color — Red channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarEnvRimG(
		TEXT("r.TrueSyncEndfieldShading.EnvRimG"), 0.82f,
		TEXT("Environment rim light color — Green channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarEnvRimB(
		TEXT("r.TrueSyncEndfieldShading.EnvRimB"), 1.00f,
		TEXT("Environment rim light color — Blue channel."), ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimIntensity(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimIntensity"), 0.18f,
		TEXT("HDR white rim on bright reflective environment pixels. Separate from shadow/sky-light edge lift."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimIntensityScale(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimIntensityScale"), 2.0f,
		TEXT("Additional scale for background white rim. Lower values reduce temporal flicker on bright repeated surfaces."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimPower(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimPower"), 2.2f,
		TEXT("Background white rim falloff exponent. Lower values make the highlight wider."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimSurfaceStrength(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimSurfaceStrength"), 0.28f,
		TEXT("How much bright reflective background surfaces can glow even when the depth/normal edge mask is weak."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimLumaStart(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimLumaStart"), 0.42f,
		TEXT("Perceptual luma where background white rim starts."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimLumaEnd(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimLumaEnd"), 0.72f,
		TEXT("Perceptual luma where background white rim reaches full strength."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimHighlightContrast(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimHighlightContrast"), 1.5f,
		TEXT("How much local bright-on-dark contrast contributes to the background white rim."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimFadeStartDistance(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimFadeStartDistance"), 3500.0f,
		TEXT("Distance where background white rim starts fading out to avoid bright repeated grid streaks."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimFadeEndDistance(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimFadeEndDistance"), 11000.0f,
		TEXT("Distance where background white rim is fully faded out."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimR(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimR"), 1.00f,
		TEXT("Background white rim color — Red channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimG(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimG"), 0.97f,
		TEXT("Background white rim color — Green channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarBackgroundWhiteRimB(
		TEXT("r.TrueSyncEndfieldShading.BackgroundWhiteRimB"), 0.90f,
		TEXT("Background white rim color — Blue channel."), ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvShadowDensity(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowDensity"), 0.40f,
		TEXT("Additional environment shadow density. Helps prevent grey, lifted shadows."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvShadowLumaStart(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowLumaStart"), 0.10f,
		TEXT("Perceptual luma where environment shadow protection is fully active."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvShadowLumaEnd(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowLumaEnd"), 0.55f,
		TEXT("Perceptual luma where environment shadow protection fades out."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvShadowHazeSuppression(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowHazeSuppression"), 0.95f,
		TEXT("How strongly haze is suppressed in dark environment pixels."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarEnvShadowR(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowR"), 0.56f,
		TEXT("Environment shadow tint — Red channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarEnvShadowG(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowG"), 0.60f,
		TEXT("Environment shadow tint — Green channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarEnvShadowB(
		TEXT("r.TrueSyncEndfieldShading.EnvShadowB"), 0.66f,
		TEXT("Environment shadow tint — Blue channel."), ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarForwardDepthRimIntensity(
		TEXT("r.TrueSyncEndfieldShading.ForwardDepthRimIntensity"), 0.08f,
		TEXT("Depth-gradient rim fallback used when GBuffer normals are unavailable."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarForwardDepthRimThreshold(
		TEXT("r.TrueSyncEndfieldShading.ForwardDepthRimThreshold"), 0.004f,
		TEXT("Relative depth threshold for the forward/mobile rim fallback."),
		ECVF_RenderThreadSafe);

	// -----------------------------------------------------------------------
	// Haze
	// -----------------------------------------------------------------------
	static TAutoConsoleVariable<float> CVarHazeStartDistance(
		TEXT("r.TrueSyncEndfieldShading.HazeStartDistance"), 3500.0f,
		TEXT("Distance where environment haze begins (cm)."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarHazeEndDistance(
		TEXT("r.TrueSyncEndfieldShading.HazeEndDistance"), 18000.0f,
		TEXT("Distance where environment haze reaches full strength (cm)."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarHazeIntensity(
		TEXT("r.TrueSyncEndfieldShading.HazeIntensity"), 0.05f,
		TEXT("Overall haze intensity. 0=off."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarHazeTintStrength(
		TEXT("r.TrueSyncEndfieldShading.HazeTintStrength"), 0.04f,
		TEXT("Strength of haze tint color influence."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarHazeTintR(
		TEXT("r.TrueSyncEndfieldShading.HazeTintR"), 0.52f,
		TEXT("Haze tint color — Red channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarHazeTintG(
		TEXT("r.TrueSyncEndfieldShading.HazeTintG"), 0.56f,
		TEXT("Haze tint color — Green channel."), ECVF_RenderThreadSafe);
	static TAutoConsoleVariable<float> CVarHazeTintB(
		TEXT("r.TrueSyncEndfieldShading.HazeTintB"), 0.62f,
		TEXT("Haze tint color — Blue channel."), ECVF_RenderThreadSafe);

	// -----------------------------------------------------------------------
	// Interior Volume
	// -----------------------------------------------------------------------
	static TAutoConsoleVariable<float> CVarInteriorHazeMultiplier(
		TEXT("r.TrueSyncEndfieldShading.InteriorHazeMultiplier"), 0.18f,
		TEXT("Haze multiplier inside a TrueSyncEndfieldInteriorVolume."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarInteriorCoolShiftMultiplier(
		TEXT("r.TrueSyncEndfieldShading.InteriorCoolShiftMultiplier"), 0.30f,
		TEXT("Cool shift multiplier inside a TrueSyncEndfieldInteriorVolume."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarInteriorDesaturationMultiplier(
		TEXT("r.TrueSyncEndfieldShading.InteriorDesaturationMultiplier"), 0.55f,
		TEXT("Desaturation multiplier inside a TrueSyncEndfieldInteriorVolume."),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<float> CVarInteriorContrast(
		TEXT("r.TrueSyncEndfieldShading.InteriorContrast"), 1.02f,
		TEXT("Contrast multiplier inside a TrueSyncEndfieldInteriorVolume."),
		ECVF_RenderThreadSafe);

	// -----------------------------------------------------------------------
	// 디버그
	// -----------------------------------------------------------------------
	static TAutoConsoleVariable<int32> CVarDebugZoneMask(
		TEXT("r.TrueSyncEndfieldShading.DebugZoneMask"), 0,
		TEXT("Visualize pixel zone classification.\n"
		     "0: Off  1: On  (Sky=Blue / Character=Red / Environment=Green)"),
		ECVF_RenderThreadSafe);

	static TAutoConsoleVariable<int32> CVarDebugBackgroundWhiteRim(
		TEXT("r.TrueSyncEndfieldShading.DebugBackgroundWhiteRim"), 0,
		TEXT("Visualize the background white rim mask.\n0: Off  1: Warm yellow mask"),
		ECVF_RenderThreadSafe);

} // namespace TrueSyncEndfieldShading

// ===========================================================================
// FTrueSyncEndfieldShadingSceneViewExtension
// ===========================================================================

FTrueSyncEndfieldShadingSceneViewExtension::FTrueSyncEndfieldShadingSceneViewExtension(const FAutoRegister& AutoRegister)
	: FSceneViewExtensionBase(AutoRegister)
	, bIsAlive(true)
{
}

void FTrueSyncEndfieldShadingSceneViewExtension::Invalidate()
{
	bIsAlive.Store(false);
}

void FTrueSyncEndfieldShadingSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	FCachedCameraZoneState CameraZoneState;

		if (UWorld* World = TrueSyncEndfieldShading::ResolveViewWorld(InView))
	{
		if (const UTrueSyncEndfieldShadingWorldSubsystem* Subsystem =
			World->GetSubsystem<UTrueSyncEndfieldShadingWorldSubsystem>())
		{
			const FTrueSyncEndfieldInteriorState InteriorState =
				Subsystem->ComputeInteriorState(InView.ViewLocation);
			CameraZoneState.InteriorBlend = InteriorState.Blend;
			CameraZoneState.InteriorHazeMultiplier = InteriorState.HazeMultiplier;
			CameraZoneState.InteriorCoolShiftMultiplier = InteriorState.CoolShiftMultiplier;
			CameraZoneState.InteriorDesaturationMultiplier = InteriorState.DesaturationMultiplier;
			CameraZoneState.InteriorContrast = InteriorState.Contrast;
		}
		else
		{
			for (TActorIterator<ATrueSyncEndfieldInteriorVolume> It(World); It; ++It)
			{
				CameraZoneState.InteriorBlend = FMath::Max(
					CameraZoneState.InteriorBlend,
					It->ComputeBlendWeight(InView.ViewLocation));
			}
		}
	}

	FScopeLock Lock(&CachedCameraZoneStateMutex);
	CachedCameraZoneState = CameraZoneState;
}

bool FTrueSyncEndfieldShadingSceneViewExtension::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	return bIsAlive.Load() && TrueSyncEndfieldShading::CVarEnable.GetValueOnAnyThread() != 0;
}

void FTrueSyncEndfieldShadingSceneViewExtension::SubscribeToPostProcessingPass(
	EPostProcessingPass PassId,
	const FSceneView& View,
	FPostProcessingPassDelegateArray& InOutPassCallbacks,
	bool bIsPassEnabled)
{
	if (!bIsPassEnabled || PassId != EPostProcessingPass::AfterDOF)
	{
		return;
	}

	if (!View.Family || !View.Family->EngineShowFlags.PostProcessing)
	{
		return;
	}

		InOutPassCallbacks.Add(FPostProcessingPassDelegate::CreateLambda(
			[this](FRDGBuilder& GraphBuilder, const FSceneView& InView, const FPostProcessMaterialInputs& Inputs)
			{
				return ApplyEndfieldPass(GraphBuilder, InView, Inputs);
			}));
	}

FScreenPassTexture FTrueSyncEndfieldShadingSceneViewExtension::ApplyEndfieldPass(
	FRDGBuilder& GraphBuilder,
	const FSceneView& View,
	const FPostProcessMaterialInputs& Inputs)
{
	if (!bIsAlive.Load() || TrueSyncEndfieldShading::CVarEnable.GetValueOnRenderThread() == 0)
	{
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	if (!Inputs.SceneTextures.SceneTextures && !Inputs.SceneTextures.MobileSceneTextures)
	{
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	const FScreenPassTexture SceneColor = FScreenPassTexture::CopyFromSlice(
		GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));

	if (!SceneColor.IsValid())
	{
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	FScreenPassRenderTarget Output = Inputs.OverrideOutput;
	if (!Output.IsValid())
	{
		Output = FScreenPassRenderTarget::CreateFromInput(
			GraphBuilder, SceneColor,
			ERenderTargetLoadAction::ENoAction, TEXT("TrueSyncEndfieldShading.Output"));
	}

	// ------------------------------------------------------------------
	// 셰이더 취득 및 유효성 검사
	// ------------------------------------------------------------------
	TShaderMapRef<FScreenPassVS> VertexShader(GetGlobalShaderMap(View.GetFeatureLevel()));
	TShaderMapRef<FTrueSyncEndfieldCompositePS> PixelShader(GetGlobalShaderMap(View.GetFeatureLevel()));

	if (!VertexShader.IsValid() || !PixelShader.IsValid())
	{
		UE_LOG(LogTrueSyncEndfieldShading, Warning,
			TEXT("TrueSyncEndfieldShading: shader is not valid for feature level %d — skipping pass. "
			     "Check for shader compile errors or stale shader cache."),
			static_cast<int32>(View.GetFeatureLevel()));
		return Inputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
	}

	// ------------------------------------------------------------------
	// 파라미터 바인딩
	// ------------------------------------------------------------------
	const FScreenPassTextureViewport InputViewport(SceneColor);
	const FScreenPassTextureViewport OutputViewport(Output);
	const FCachedCameraZoneState CameraZoneState = GetCachedCameraZoneState();

	FTrueSyncEndfieldCompositePS::FParameters* PassParameters =
		GraphBuilder.AllocParameters<FTrueSyncEndfieldCompositePS::FParameters>();

	PassParameters->View             = View.ViewUniformBuffer;
	PassParameters->SceneTextures    = Inputs.SceneTextures;
	PassParameters->InputViewport    = GetScreenPassTextureViewportParameters(InputViewport);
	PassParameters->InputSceneColorTexture = SceneColor.Texture;
	PassParameters->InputSceneColorSampler =
		TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	// 존 분류
	PassParameters->SkyDepthThreshold     = TrueSyncEndfieldShading::CVarSkyDepthThreshold.GetValueOnRenderThread();
	PassParameters->CharacterStencilEnabled =
		TrueSyncEndfieldShading::CVarCharacterStencilEnable.GetValueOnRenderThread() != 0 ? 1.0f : 0.0f;
	PassParameters->CharacterStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarCharacterStencilValue.GetValueOnRenderThread());
	PassParameters->SkinStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarSkinStencilValue.GetValueOnRenderThread());
	PassParameters->FaceStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarFaceStencilValue.GetValueOnRenderThread());
	PassParameters->HairStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarHairStencilValue.GetValueOnRenderThread());
	PassParameters->ClothStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarClothStencilValue.GetValueOnRenderThread());
	PassParameters->MetalGearStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarMetalGearStencilValue.GetValueOnRenderThread());
	PassParameters->TerrainStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarTerrainStencilValue.GetValueOnRenderThread());
	PassParameters->IndustrialMetalStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarIndustrialMetalStencilValue.GetValueOnRenderThread());
	PassParameters->ConcreteStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarConcreteStencilValue.GetValueOnRenderThread());
	PassParameters->VFXStencilValue =
		static_cast<float>(TrueSyncEndfieldShading::CVarVFXStencilValue.GetValueOnRenderThread());
		PassParameters->UseMaterialStencilPalette =
			TrueSyncEndfieldShading::CVarUseMaterialStencilPalette.GetValueOnRenderThread() != 0 ? 1.0f : 0.0f;
		PassParameters->CustomDepthCharacterFallback =
			TrueSyncEndfieldShading::CVarCustomDepthCharacterFallback.GetValueOnRenderThread() != 0 ? 1.0f : 0.0f;
		PassParameters->CustomDepthFallbackThreshold =
			TrueSyncEndfieldShading::CVarCustomDepthFallbackThreshold.GetValueOnRenderThread();
		PassParameters->KeyLightDirection = TrueSyncEndfieldShading::GetKeyLightDirectionOnRenderThread();

		// Zone B — 캐릭터
	PassParameters->CharacterContrastBoost   = TrueSyncEndfieldShading::CVarCharacterContrastBoost.GetValueOnRenderThread();
	PassParameters->CharacterSaturationBoost = TrueSyncEndfieldShading::CVarCharacterSaturationBoost.GetValueOnRenderThread();
	PassParameters->CharacterCelShadingStrength =
		TrueSyncEndfieldShading::CVarCharacterCelShadingStrength.GetValueOnRenderThread();
	PassParameters->CharacterCelShadowThreshold =
		TrueSyncEndfieldShading::CVarCharacterCelShadowThreshold.GetValueOnRenderThread();
	PassParameters->CharacterCelShadowSoftness =
		TrueSyncEndfieldShading::CVarCharacterCelShadowSoftness.GetValueOnRenderThread();
	PassParameters->CharacterCelShadowDepth =
		TrueSyncEndfieldShading::CVarCharacterCelShadowDepth.GetValueOnRenderThread();
	PassParameters->CharacterCelShadowTint = FVector3f(
		TrueSyncEndfieldShading::CVarCharacterCelShadowR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarCharacterCelShadowG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarCharacterCelShadowB.GetValueOnRenderThread());
	PassParameters->CharacterRimIntensity    = TrueSyncEndfieldShading::CVarCharacterRimIntensity.GetValueOnRenderThread();
	PassParameters->CharacterRimPower        = TrueSyncEndfieldShading::CVarCharacterRimPower.GetValueOnRenderThread();
	PassParameters->CharacterDirectionalRimStrength =
		TrueSyncEndfieldShading::CVarCharacterDirectionalRimStrength.GetValueOnRenderThread();
	PassParameters->CharacterDirectionalRimLightStart =
		TrueSyncEndfieldShading::CVarCharacterDirectionalRimLightStart.GetValueOnRenderThread();
	PassParameters->CharacterDirectionalRimLightEnd =
		TrueSyncEndfieldShading::CVarCharacterDirectionalRimLightEnd.GetValueOnRenderThread();
	PassParameters->CharacterDirectionalRimLightWrap =
		TrueSyncEndfieldShading::CVarCharacterDirectionalRimLightWrap.GetValueOnRenderThread();
		PassParameters->CharacterRimColor        = FVector3f(
			TrueSyncEndfieldShading::CVarCharacterRimR.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterRimG.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterRimB.GetValueOnRenderThread());
		PassParameters->CharacterWhiteRimIntensity =
			TrueSyncEndfieldShading::CVarCharacterWhiteRimIntensity.GetValueOnRenderThread();
		PassParameters->CharacterWhiteRimPower =
			TrueSyncEndfieldShading::CVarCharacterWhiteRimPower.GetValueOnRenderThread();
		PassParameters->CharacterWhiteRimColor = FVector3f(
			TrueSyncEndfieldShading::CVarCharacterWhiteRimR.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterWhiteRimG.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterWhiteRimB.GetValueOnRenderThread());
		PassParameters->CharacterOutlineIntensity =
			TrueSyncEndfieldShading::CVarCharacterOutlineIntensity.GetValueOnRenderThread();
		PassParameters->CharacterOutlineWidth =
			TrueSyncEndfieldShading::CVarCharacterOutlineWidth.GetValueOnRenderThread();
		PassParameters->CharacterOutlineColor = FVector3f(
			TrueSyncEndfieldShading::CVarCharacterOutlineR.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterOutlineG.GetValueOnRenderThread(),
			TrueSyncEndfieldShading::CVarCharacterOutlineB.GetValueOnRenderThread());

		// Zone C — 환경
	PassParameters->EnvDesaturation = TrueSyncEndfieldShading::CVarEnvDesaturation.GetValueOnRenderThread();
	PassParameters->EnvCoolShift    = TrueSyncEndfieldShading::CVarEnvCoolShift.GetValueOnRenderThread();
	PassParameters->EnvCelShadingStrength =
		TrueSyncEndfieldShading::CVarEnvCelShadingStrength.GetValueOnRenderThread();
	PassParameters->EnvCelShadowThreshold =
		TrueSyncEndfieldShading::CVarEnvCelShadowThreshold.GetValueOnRenderThread();
	PassParameters->EnvCelShadowSoftness =
		TrueSyncEndfieldShading::CVarEnvCelShadowSoftness.GetValueOnRenderThread();
	PassParameters->EnvCelShadowDepth =
		TrueSyncEndfieldShading::CVarEnvCelShadowDepth.GetValueOnRenderThread();
	PassParameters->EnvCelShadowTint = FVector3f(
		TrueSyncEndfieldShading::CVarEnvCelShadowR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvCelShadowG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvCelShadowB.GetValueOnRenderThread());
	PassParameters->EnvCelFadeStartDistance =
		TrueSyncEndfieldShading::CVarEnvCelFadeStartDistance.GetValueOnRenderThread();
	PassParameters->EnvCelFadeEndDistance =
		TrueSyncEndfieldShading::CVarEnvCelFadeEndDistance.GetValueOnRenderThread();
	PassParameters->EnvRimIntensity = TrueSyncEndfieldShading::CVarEnvRimIntensity.GetValueOnRenderThread();
	PassParameters->EnvRimPower     = TrueSyncEndfieldShading::CVarEnvRimPower.GetValueOnRenderThread();
	PassParameters->EnvRimColor     = FVector3f(
		TrueSyncEndfieldShading::CVarEnvRimR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvRimG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvRimB.GetValueOnRenderThread());
	PassParameters->BackgroundWhiteRimIntensity =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimIntensity.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimIntensityScale =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimIntensityScale.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimPower =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimPower.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimSurfaceStrength =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimSurfaceStrength.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimLumaStart =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimLumaStart.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimLumaEnd =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimLumaEnd.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimHighlightContrast =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimHighlightContrast.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimFadeStartDistance =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimFadeStartDistance.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimFadeEndDistance =
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimFadeEndDistance.GetValueOnRenderThread();
	PassParameters->BackgroundWhiteRimColor = FVector3f(
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarBackgroundWhiteRimB.GetValueOnRenderThread());
	PassParameters->EnvShadowDensity         = TrueSyncEndfieldShading::CVarEnvShadowDensity.GetValueOnRenderThread();
	PassParameters->EnvShadowLumaStart       = TrueSyncEndfieldShading::CVarEnvShadowLumaStart.GetValueOnRenderThread();
	PassParameters->EnvShadowLumaEnd         = TrueSyncEndfieldShading::CVarEnvShadowLumaEnd.GetValueOnRenderThread();
	PassParameters->EnvShadowHazeSuppression = TrueSyncEndfieldShading::CVarEnvShadowHazeSuppression.GetValueOnRenderThread();
	PassParameters->EnvShadowTint            = FVector3f(
		TrueSyncEndfieldShading::CVarEnvShadowR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvShadowG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarEnvShadowB.GetValueOnRenderThread());
	PassParameters->ForwardDepthRimIntensity = TrueSyncEndfieldShading::CVarForwardDepthRimIntensity.GetValueOnRenderThread();
	PassParameters->ForwardDepthRimThreshold = TrueSyncEndfieldShading::CVarForwardDepthRimThreshold.GetValueOnRenderThread();

	// Haze
	PassParameters->HazeStartDistance = TrueSyncEndfieldShading::CVarHazeStartDistance.GetValueOnRenderThread();
	PassParameters->HazeEndDistance   = TrueSyncEndfieldShading::CVarHazeEndDistance.GetValueOnRenderThread();
	PassParameters->HazeIntensity     = TrueSyncEndfieldShading::CVarHazeIntensity.GetValueOnRenderThread();
	PassParameters->HazeTintStrength  = TrueSyncEndfieldShading::CVarHazeTintStrength.GetValueOnRenderThread();
	PassParameters->HazeTint          = FVector3f(
		TrueSyncEndfieldShading::CVarHazeTintR.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarHazeTintG.GetValueOnRenderThread(),
		TrueSyncEndfieldShading::CVarHazeTintB.GetValueOnRenderThread());

	// Interior Volume
	PassParameters->InteriorBlend                   = CameraZoneState.InteriorBlend;
	PassParameters->InteriorHazeMultiplier          = FMath::Lerp(
		TrueSyncEndfieldShading::CVarInteriorHazeMultiplier.GetValueOnRenderThread(),
		CameraZoneState.InteriorHazeMultiplier,
		CameraZoneState.InteriorBlend);
	PassParameters->InteriorCoolShiftMultiplier     = FMath::Lerp(
		TrueSyncEndfieldShading::CVarInteriorCoolShiftMultiplier.GetValueOnRenderThread(),
		CameraZoneState.InteriorCoolShiftMultiplier,
		CameraZoneState.InteriorBlend);
	PassParameters->InteriorDesaturationMultiplier  = FMath::Lerp(
		TrueSyncEndfieldShading::CVarInteriorDesaturationMultiplier.GetValueOnRenderThread(),
		CameraZoneState.InteriorDesaturationMultiplier,
		CameraZoneState.InteriorBlend);
	PassParameters->InteriorContrast                = FMath::Lerp(
		TrueSyncEndfieldShading::CVarInteriorContrast.GetValueOnRenderThread(),
		CameraZoneState.InteriorContrast,
		CameraZoneState.InteriorBlend);

	// 디버그
	PassParameters->DebugZoneMask =
		TrueSyncEndfieldShading::CVarDebugZoneMask.GetValueOnRenderThread() != 0 ? 1.0f : 0.0f;
	PassParameters->DebugBackgroundWhiteRim =
		TrueSyncEndfieldShading::CVarDebugBackgroundWhiteRim.GetValueOnRenderThread() != 0 ? 1.0f : 0.0f;

	PassParameters->RenderTargets[0] = Output.GetRenderTargetBinding();

	// ------------------------------------------------------------------
	// 드로우
	// ------------------------------------------------------------------
	AddDrawScreenPass(
		GraphBuilder,
		RDG_EVENT_NAME("TrueSyncEndfieldShading_Endfield"),
		View,
		OutputViewport,
		InputViewport,
		VertexShader,
		PixelShader,
		PassParameters);

	return MoveTemp(Output);
}

FTrueSyncEndfieldShadingSceneViewExtension::FCachedCameraZoneState
FTrueSyncEndfieldShadingSceneViewExtension::GetCachedCameraZoneState() const
{
	FScopeLock Lock(&CachedCameraZoneStateMutex);
	return CachedCameraZoneState;
}
