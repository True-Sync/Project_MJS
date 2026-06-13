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


	이런식으로 사용해주세용.(동후니 태허니 화이또) - 용석이가
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

private:
	bool CanPlayEvent(UFMODEvent* Event, float Cooldown);
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

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UFMODAudioComponent>> AttachedSFXMap;

	TMap<TObjectKey<UFMODEvent>, float> LastPlayTimes;
};
