#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrueSyncEndfieldInteriorVolume.generated.h"

class UBoxComponent;
class UTrueSyncEndfieldShadingProfile;

UCLASS(BlueprintType, Blueprintable)
class TRUESYNCENDFIELDSHADING_API ATrueSyncEndfieldInteriorVolume : public AActor
{
	GENERATED_BODY()

public:
	ATrueSyncEndfieldInteriorVolume();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

	UFUNCTION(BlueprintCallable, Category = "TrueSync Endfield Shading")
	float ComputeBlendWeight(const FVector& WorldLocation) const;

	const UTrueSyncEndfieldShadingProfile* GetProfile() const { return Profile; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TrueSync Endfield Shading")
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading", meta = (ClampMin = "0.0"))
	float BlendDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrueSync Endfield Shading")
	TObjectPtr<UTrueSyncEndfieldShadingProfile> Profile;

private:
	void RegisterWithSubsystem();
	void UnregisterWithSubsystem();
	bool bRegisteredWithSubsystem = false;
};
