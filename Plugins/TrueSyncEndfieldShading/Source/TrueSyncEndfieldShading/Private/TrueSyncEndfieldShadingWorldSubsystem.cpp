#include "TrueSyncEndfieldShadingWorldSubsystem.h"

#include "TrueSyncEndfieldInteriorVolume.h"
#include "TrueSyncEndfieldShadingProfile.h"

void UTrueSyncEndfieldShadingWorldSubsystem::RegisterInteriorVolume(ATrueSyncEndfieldInteriorVolume* Volume)
{
	if (!IsValid(Volume))
	{
		return;
	}

	InteriorVolumes.RemoveAllSwap([](const TWeakObjectPtr<ATrueSyncEndfieldInteriorVolume>& Entry)
	{
		return !Entry.IsValid();
	});

	InteriorVolumes.AddUnique(Volume);
}

void UTrueSyncEndfieldShadingWorldSubsystem::UnregisterInteriorVolume(ATrueSyncEndfieldInteriorVolume* Volume)
{
	InteriorVolumes.RemoveAllSwap([Volume](const TWeakObjectPtr<ATrueSyncEndfieldInteriorVolume>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Volume;
	});
}

FTrueSyncEndfieldInteriorState UTrueSyncEndfieldShadingWorldSubsystem::ComputeInteriorState(
	const FVector& WorldLocation) const
{
	FTrueSyncEndfieldInteriorState State;

	float TotalWeight = 0.0f;
	float HazeSum = 0.0f;
	float CoolShiftSum = 0.0f;
	float DesaturationSum = 0.0f;
	float ContrastSum = 0.0f;

	for (const TWeakObjectPtr<ATrueSyncEndfieldInteriorVolume>& Entry : InteriorVolumes)
	{
		const ATrueSyncEndfieldInteriorVolume* Volume = Entry.Get();
		if (!IsValid(Volume))
		{
			continue;
		}

		const float Weight = Volume->ComputeBlendWeight(WorldLocation);
		if (Weight <= UE_KINDA_SMALL_NUMBER)
		{
			continue;
		}

		const UTrueSyncEndfieldShadingProfile* Profile = Volume->GetProfile();
		const float HazeMultiplier = Profile ? Profile->HazeMultiplier : 0.18f;
		const float CoolShiftMultiplier = Profile ? Profile->CoolShiftMultiplier : 0.30f;
		const float DesaturationMultiplier = Profile ? Profile->DesaturationMultiplier : 0.55f;
		const float Contrast = Profile ? Profile->Contrast : 1.02f;

		State.Blend = FMath::Max(State.Blend, Weight);
		TotalWeight += Weight;
		HazeSum += HazeMultiplier * Weight;
		CoolShiftSum += CoolShiftMultiplier * Weight;
		DesaturationSum += DesaturationMultiplier * Weight;
		ContrastSum += Contrast * Weight;
	}

	if (TotalWeight > UE_KINDA_SMALL_NUMBER)
	{
		const float InvWeight = 1.0f / TotalWeight;
		State.HazeMultiplier = HazeSum * InvWeight;
		State.CoolShiftMultiplier = CoolShiftSum * InvWeight;
		State.DesaturationMultiplier = DesaturationSum * InvWeight;
		State.Contrast = ContrastSum * InvWeight;
	}

	return State;
}
