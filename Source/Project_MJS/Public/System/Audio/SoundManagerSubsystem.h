#pragma once

#include "CoreMinimal.h"
#include "FMODBlueprintStatics.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/ObjectKey.h"
#include "SoundManagerSubsystem.generated.h"

class USoundDataAsset;
class UFMODEvent;
class UFMODAudioComponent;
class UFMODBus;
class USceneComponent;

UENUM(BlueprintType)
enum class ESoundBGMState : uint8
{
	None,
	Exploration,
	Combat,
	Boss,
	Victory,
	Death
};

/*
  FMOD Sound 서브시스템 사용방법(펼쳐서 확인해주셔용)

  사용 예시 세팅 법부터 -> 사용방법까지
================================================================================================
  1. GameInstance 세팅
------------------------------------------------------------------------------------------------
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USoundManagerSubsystem* SoundMgr = GI->GetSubsystem<USoundManagerSubsystem>())
		{
			SoundMgr->SetSoundData(SoundDataAsset);
			SoundMgr->PlayBGM(SoundDataAsset->Music.GameplayBGM);
		}
	}
================================================================================================
  2. 공격음
------------------------------------------------------------------------------------------------
	SoundMgr->PlaySFXAtLocation(SoundDataAsset->Player.AttackSwing, GetActorLocation());
================================================================================================
  3. 스킬 루프시
------------------------------------------------------------------------------------------------
	SoundMgr->PlaySFXAttached(
		TEXT("PlayerSkillLoop"),
		SoundDataAsset->Player.SkillLoop,
		GetRootComponent());
================================================================================================
  4. 스킬 종료시
------------------------------------------------------------------------------------------------
	SoundMgr->StopAttachedSFX(TEXT("PlayerSkillLoop"));
	SoundMgr->PlaySFXAtLocation(SoundDataAsset->Player.SkillEnd, GetActorLocation());
================================================================================================
  5. BGM 파라미터
------------------------------------------------------------------------------------------------
	SoundMgr->SetBGMParameter(TEXT("CombatState"), 1.0f);
	SoundMgr->SetBGMParameter(TEXT("BossPhase"), 2.0f);
================================================================================================


	이런식으로 사용해주셔요
*/
UCLASS()
class PROJECT_MJS_API USoundManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void SetSoundData(USoundDataAsset* InSoundData);

	UFUNCTION(BlueprintPure, Category = "Sound")
	USoundDataAsset* GetSoundData() const { return SoundData; }

	// ======== BGM ========
	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void PlayBGM(UFMODEvent* Event, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void StopBGM(bool bAllowFadeOut = true);

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void PauseBGM(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void SetBGMParameter(FName ParameterName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void SetBGMState(ESoundBGMState NewState, float Volume = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Sound|BGM")
	ESoundBGMState GetBGMState() const { return CurrentBGMState; }

	UFUNCTION(BlueprintCallable, Category = "Sound|BGM")
	void SetBGMIntensity(float Intensity);

	// ======== SFX ========
	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	void PlaySFX2D(UFMODEvent* Event, float Volume = 1.0f, float Cooldown = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	void PlaySFXAtLocation(UFMODEvent* Event, FVector Location, float Volume = 1.0f, float Cooldown = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	UFMODAudioComponent* PlaySFXAttached(
		FName Handle,
		UFMODEvent* Event,
		USceneComponent* AttachToComponent,
		FName AttachPointName = NAME_None,
		float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|SFX")
	void StopAttachedSFX(FName Handle);

	// ======== Ambience ========
	UFUNCTION(BlueprintCallable, Category = "Sound|Ambience")
	void PlayAmbience(UFMODEvent* Event, float Volume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|Ambience")
	void StopAmbience(bool bAllowFadeOut = true);

	UFUNCTION(BlueprintCallable, Category = "Sound|Ambience")
	void PauseAmbience(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Sound|Ambience")
	void SetAmbienceParameter(FName ParameterName, float Value);

	// ======== Bus / Mix ========
	UFUNCTION(BlueprintCallable, Category = "Sound|Mix")
	void ApplyVolumes(
		float MasterVolume,
		float BGMVolume,
		float SFXVolume,
		float UIVolume,
		float VoiceVolume = 1.0f,
		float AmbienceVolume = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Sound|Mix")
	void SetSFXPaused(bool bPaused);

	UFUNCTION(BlueprintCallable, Category = "Sound|Mix")
	void LoadVolumeSettings();

	UFUNCTION(BlueprintCallable, Category = "Sound|Mix")
	void SaveVolumeSettings() const;

	// ======== Pause ========
	UFUNCTION(BlueprintCallable, Category = "Sound|Pause")
	void ApplyPauseAudio(float PauseBGMVolume = 0.4f);

	UFUNCTION(BlueprintCallable, Category = "Sound|Pause")
	void RestorePauseAudio();

	// ======== Debug ========
	UFUNCTION(BlueprintCallable, Category = "Sound|Debug")
	void SetDebugSoundEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Sound|Debug")
	void DumpActiveSounds() const;

private:
	bool CanPlayEvent(UFMODEvent* Event, float Cooldown);
	UFMODEvent* GetBGMEventForState(ESoundBGMState State) const;
	void StartPersistentInstance(FFMODEventInstance& Instance, UFMODEvent* Event, float Volume);
	void StopPersistentInstance(FFMODEventInstance& Instance, bool bAllowFadeOut);
	void SetBusVolume(UFMODBus* Bus, float Volume);
	void SetBusPaused(UFMODBus* Bus, bool bPaused);
	void StopAllAttachedSFX();

private:
	UPROPERTY(Transient)
	TObjectPtr<USoundDataAsset> SoundData = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UFMODEvent> CurrentBGMEvent = nullptr;

	FFMODEventInstance BGMInstance;
	bool bHasBGMInstance = false;
	ESoundBGMState CurrentBGMState = ESoundBGMState::None;

	UPROPERTY(Transient)
	TObjectPtr<UFMODEvent> CurrentAmbienceEvent = nullptr;

	FFMODEventInstance AmbienceInstance;
	bool bHasAmbienceInstance = false;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFMODAudioComponent>> AttachedSFXMap;

	TMap<TObjectKey<UFMODEvent>, float> LastPlayTimes;

	float CurrentMasterVolume = 1.0f;
	float CurrentBGMVolume = 1.0f;
	float CurrentSFXVolume = 1.0f;
	float CurrentUIVolume = 1.0f;
	float CurrentVoiceVolume = 1.0f;
	float CurrentAmbienceVolume = 1.0f;

	float PrePauseBGMVolume = 1.0f;
	bool bPauseAudioApplied = false;
	bool bDebugSound = false;
};
