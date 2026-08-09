#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HousingSubsystem.generated.h"

class ACPlayerController;
class ACommandBoxActor;
class AHousingAreaActor;

UCLASS()
class PROJECT_MJS_API UHousingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void HandleCommandBoxHousingRequested(ACommandBoxActor* CommandBox, ACPlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Housing")
	bool EnterHousing(ACPlayerController* PlayerController, AHousingAreaActor* HousingArea);

	UFUNCTION(BlueprintCallable, Category = "Housing")
	void ExitHousing(ACPlayerController* PlayerController);
};
