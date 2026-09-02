#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "HousingCameraActor.generated.h"

UCLASS()
class PROJECT_MJS_API AHousingCameraActor : public ACameraActor
{
	GENERATED_BODY()

public:
	AHousingCameraActor();

	float GetBlendTime() const { return BlendTime; }

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BlendTime = 0.35f;
};
