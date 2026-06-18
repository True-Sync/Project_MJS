# TrueSync Endfield Material Graph Usage

The post-process pass now provides global cel bands and directional character rim.
Per-material tuning can be layered in material graphs with Custom nodes that include
the public shader helpers.

## Include

Use this include in a material Custom node:

```hlsl
#include "/Plugin/TrueSyncEndfieldShading/Public/TrueSyncEndfieldMaterialGraphCommon.ush"
```

You can also include the character/environment wrappers:

```hlsl
#include "/Plugin/TrueSyncEndfieldShading/Public/TrueSyncEndfieldCharacterCommon.ush"
#include "/Plugin/TrueSyncEndfieldShading/Public/TrueSyncEndfieldEnvironmentCommon.ush"
```

## Character Custom Node

Return type: `CMOT Float 3`

Inputs:

- `BaseColor` float3
- `NormalWS` float3
- `ViewDirWS` float3
- `LightDirWS` float3
- `CelStrength` float
- `CelThreshold` float
- `CelSoftness` float
- `CelShadowDepth` float
- `CelShadowTint` float3
- `RimColor` float3
- `RimIntensity` float
- `RimPower` float
- `RimLightStart` float
- `RimLightEnd` float
- `RimLightWrap` float
- `DirectionalRimStrength` float

Code:

```hlsl
#include "/Plugin/TrueSyncEndfieldShading/Public/TrueSyncEndfieldCharacterCommon.ush"

return TSE_CharacterGraphCelRim(
	BaseColor,
	NormalWS,
	ViewDirWS,
	LightDirWS,
	CelStrength,
	CelThreshold,
	CelSoftness,
	CelShadowDepth,
	CelShadowTint,
	RimColor,
	RimIntensity,
	RimPower,
	RimLightStart,
	RimLightEnd,
	RimLightWrap,
	DirectionalRimStrength);
```

## Environment Custom Node

Return type: `CMOT Float 3`

Inputs:

- `BaseColor` float3
- `NormalWS` float3
- `LightDirWS` float3
- `CelStrength` float
- `CelThreshold` float
- `CelSoftness` float
- `CelShadowDepth` float
- `CelShadowTint` float3

Code:

```hlsl
#include "/Plugin/TrueSyncEndfieldShading/Public/TrueSyncEndfieldEnvironmentCommon.ush"

return TSE_EnvironmentGraphCel(
	BaseColor,
	NormalWS,
	LightDirWS,
	CelStrength,
	CelThreshold,
	CelSoftness,
	CelShadowDepth,
	CelShadowTint);
```

## Suggested Graph Sources

- `NormalWS`: use a normalized world-space normal.
- `ViewDirWS`: use camera-vector/world-position direction, normalized from surface toward camera.
- `LightDirWS`: use a Material Parameter Collection vector shared with
  `r.TrueSyncEndfieldShading.KeyLightDirX/Y/Z`, or expose a vector parameter per material.
- Keep material-side cel/rim subtle when the post-process pass is enabled, so the two layers do not double-darken.
