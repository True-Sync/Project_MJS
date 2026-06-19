#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "System/Audio/SoundManagerSubsystem.h"
#include "ProjectGameInstance.generated.h"

class USoundDataAsset;

UCLASS(Blueprintable)
class PROJECT_MJS_API UProjectGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void OnStart() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void InitializeSound();

	UFUNCTION(BlueprintPure, Category = "Sound")
	USoundDataAsset* GetSoundDataAsset() const { return SoundDataAsset; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundDataAsset> SoundDataAsset = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	bool bEnableSoundDebug = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|BGM")
	bool bStartInitialBGM = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|BGM", meta = (EditCondition = "bStartInitialBGM"))
	ESoundBGMState InitialBGMState = ESoundBGMState::Exploration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound|BGM", meta = (EditCondition = "bStartInitialBGM", ClampMin = "0.0", ClampMax = "1.0"))
	float InitialBGMVolume = 1.0f;

private:
	bool bSoundInitialized = false;
};
