#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TrueSyncEndfieldShadingWorldSubsystem.generated.h"

class ATrueSyncEndfieldInteriorVolume;

struct FTrueSyncEndfieldInteriorState
{
	float Blend = 0.0f;
	float HazeMultiplier = 1.0f;
	float CoolShiftMultiplier = 1.0f;
	float DesaturationMultiplier = 1.0f;
	float Contrast = 1.0f;
};

UCLASS()
class TRUESYNCENDFIELDSHADING_API UTrueSyncEndfieldShadingWorldSubsystem final : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterInteriorVolume(ATrueSyncEndfieldInteriorVolume* Volume);
	void UnregisterInteriorVolume(ATrueSyncEndfieldInteriorVolume* Volume);

	FTrueSyncEndfieldInteriorState ComputeInteriorState(const FVector& WorldLocation) const;

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ATrueSyncEndfieldInteriorVolume>> InteriorVolumes;
};
