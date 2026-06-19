#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SoundDataAsset.generated.h"

class UFMODEvent;
class UFMODBus;

USTRUCT(BlueprintType)
struct FMusicEvents
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Music")
	TObjectPtr<UFMODEvent> GameplayBGM = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Music")
	TObjectPtr<UFMODEvent> BattleBGM = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Music")
	TObjectPtr<UFMODEvent> BossBGM = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Music")
	TObjectPtr<UFMODEvent> VictoryBGM = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Music")
	TObjectPtr<UFMODEvent> DeathBGM = nullptr;
};

USTRUCT(BlueprintType)
struct FUIEvents
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|UI")
	TObjectPtr<UFMODEvent> Hover = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|UI")
	TObjectPtr<UFMODEvent> Click = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|UI")
	TObjectPtr<UFMODEvent> Confirm = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|UI")
	TObjectPtr<UFMODEvent> Cancel = nullptr;
};

USTRUCT(BlueprintType)
struct FAmbienceEvents
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Ambience")
	TObjectPtr<UFMODEvent> DefaultAmbience = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Ambience")
	TObjectPtr<UFMODEvent> Wind = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Ambience")
	TObjectPtr<UFMODEvent> Rain = nullptr;
};

USTRUCT(BlueprintType)
struct FPlayerSoundEvents
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> AttackSwing = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> AttackHit = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> Dash = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> SkillStart = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> SkillLoop = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> SkillEnd = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> Footstep = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Player")
	TObjectPtr<UFMODEvent> Land = nullptr;
};

USTRUCT(BlueprintType)
struct FFmodBuses
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> Master = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> BGM = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> SFX = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> UI = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> Voice = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD|Bus")
	TObjectPtr<UFMODBus> Ambience = nullptr;
};

UCLASS(BlueprintType)
class PROJECT_MJS_API USoundDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD")
	FMusicEvents Music;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD")
	FPlayerSoundEvents Player;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD")
	FUIEvents UI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD")
	FAmbienceEvents Ambience;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FMOD")
	FFmodBuses Buses;
};
