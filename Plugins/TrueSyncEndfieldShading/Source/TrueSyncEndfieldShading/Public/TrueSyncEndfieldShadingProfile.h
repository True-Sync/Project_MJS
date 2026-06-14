#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TrueSyncEndfieldShadingProfile.generated.h"

UCLASS(BlueprintType)
class TRUESYNCENDFIELDSHADING_API UTrueSyncEndfieldShadingProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading|Interior", meta = (ClampMin = "0.0"))
	float HazeMultiplier = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading|Interior", meta = (ClampMin = "0.0"))
	float CoolShiftMultiplier = 0.30f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading|Interior", meta = (ClampMin = "0.0"))
	float DesaturationMultiplier = 0.55f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading|Interior", meta = (ClampMin = "0.0"))
	float Contrast = 1.02f;
};
