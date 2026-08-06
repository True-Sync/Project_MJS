#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatusLayerWidget.generated.h"

class UHealthComponent;
class UProgressBar;

UCLASS()
class PROJECT_MJS_API UPlayerStatusLayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitHealthStatus(UHealthComponent* InHealthComp);

protected:
	UFUNCTION()
	void HandleHealthChanged(float OldHealth, float NewHealth);
	
	UFUNCTION()
	void HandleCatchUpTick();

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_HP;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ProgressBar_HPCatchUp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Health CatchUp")
	float CatchUpDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Health CatchUp")
	float CatchUpSpeed = 3.0f;

private:
	TWeakObjectPtr<UHealthComponent> CachedHealthComp;
	
	FTimerHandle CatchUpTimerHandle;

	float TargetHealthPercent = 1.0f;
	float CurrentCatchUpPercent = 1.0f;
	float CatchUpTimer = 0.0f;
};