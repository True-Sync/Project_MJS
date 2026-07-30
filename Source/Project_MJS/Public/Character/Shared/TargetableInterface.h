#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetableInterface.generated.h"

UINTERFACE(BlueprintType)
class PROJECT_MJS_API UTargetableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECT_MJS_API ITargetableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	FVector GetTargetPointLocation() const;
};
