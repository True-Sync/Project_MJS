#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimMontage.h"
#include "Character/SharedData/StaminaCostData.h"
#include "Components/ActorComponent.h"
#include "System/Combat/CombatTimeDilationSubsystem.h"
#include "DodgeComponent.generated.h"

class UCameraShakeBase;
class UNiagaraSystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_MJS_API UDodgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDodgeComponent();

	bool RequestDodge();
	bool IsDodging() const { return bIsDodging; }
	bool IsDodgeInvincible() const { return bIsDodging; }
	bool TryConsumeJustDodge(AActor* AttackCauser);
	bool IsJustDodgeCounterAvailable() const { return bCanJustDodgeCounter; }
	bool ConsumeJustDodgeCounter();
	
private:
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void HandleJustDodgeSucceeded(AActor* AttackCauser);
	void PlayJustDodgeCameraFeedback() const;
	void ApplyJustDodgeTimeFeedback(AActor* AttackCauser);
	void OpenJustDodgeCounterWindow();
	void CloseJustDodgeCounterWindow();

	// ===== 회피 몽타주 =====
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> DefaultDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> BackStepDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> JustDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Resource")
	FStaminaCostData DodgeStaminaCost;

	// ===== 저스트 회피 =====
	// 저스트 회피 사용 여부 (꺼도 일반 회피 무적은 유지됨)
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just")
	bool bEnableJustDodge = true;

	// 회피 입력 후 이 시간 안에 적 공격이 들어오면 저스트 회피로 인정
	// 너무 짧게 설정하면 공격 판정 타이밍에 따라 저스트 회피가 거의 성공하지 않을 수 있음
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just", meta = (ClampMin = "0.0"))
	float JustDodgeWindow = 0.28f;

	// 회피 입력 직후 너무 빠르게 들어온 공격을 제외하고 싶을 때 사용하는 최소 경과 시간
	// 이 값이 JustDodgeWindow보다 크면 저스트 회피가 절대 성공하지 않음
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just", meta = (ClampMin = "0.0"))
	float JustDodgeMinElapsed = 0.0f;

	// ===== 저스트 회피 이펙트 ===== (해당 부분은 그냥 변수 선언만 했습니다.)
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Effect")
	bool bUseJustDodgeEffect = false;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Effect")
	TObjectPtr<UNiagaraSystem> JustDodgeEffect;

	// ===== 저스트 회피 반격 ===== (상태 준비까지면 구현되어 있음)
	// 저스트 회피 성공 후 반격 가능 상태를 열지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Counter")
	bool bEnableJustDodgeCounter = true;

	// 저스트 회피 반격 가능 상태가 유지되는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Counter", meta = (ClampMin = "0.0"))
	float JustDodgeCounterWindow = 0.6f;

	// ===== 저스트 회피 카메라 연출 =====
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera")
	bool bPlayJustDodgeCameraShake = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera")
	TSubclassOf<UCameraShakeBase> JustDodgeCameraShake;

	// 저스트 회피 카메라 쉐이크 세기
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera", meta = (ClampMin = "0.0"))
	float JustDodgeCameraShakeScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera")
	bool bPlayJustDodgeFOV = true;

	// 저스트 회피 연출 FOV
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera", meta = (ClampMin = "1.0", ClampMax = "179.0"))
	float JustDodgeFOV = 90.0f;

	// 현재 FOV에서 목표 FOV까지 도달하는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera", meta = (ClampMin = "0.0"))
	float JustDodgeFOVBlendInTime = 0.04f;

	// 목표 FOV를 유지하는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera", meta = (ClampMin = "0.0"))
	float JustDodgeFOVHoldTime = 0.06f;

	// 기본 FOV로 돌아가는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Camera", meta = (ClampMin = "0.0"))
	float JustDodgeFOVBlendOutTime = 0.12f;

	// ===== 저스트 회피 시간 연출 =====
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Time")
	bool bUseJustDodgeTimeFeedback = false;

	// 저스트 회피 시간 연출 방식 선택 설정
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Time")
	ECombatTimeDilationFeedbackMode JustDodgeTimeFeedbackMode = ECombatTimeDilationFeedbackMode::HitStop;

	// 저스트 회피 HitStop 설정 (기본 대상은 플레이어와 공격자)
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Time")
	FHitStopSettings JustDodgeHitStopSettings;

	// 저스트 회피 WorldSlow 설정
	UPROPERTY(EditDefaultsOnly, Category = "Dodge|Just|Time")
	FWorldSlowSettings JustDodgeWorldSlowSettings;

	// ===== 일반 회피 상태 =====
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveDodgeMontage;

	bool bIsDodging = false;

	// ===== 저스트 회피 상태 =====
	// 회피 몽타주가 정상 시작된 순간의 월드 시간
	UPROPERTY(Transient)
	float LastDodgeInputTime = -999.0f;

	// 한 번의 회피에서 저스트 회피가 중복 성공하지 않도록 막는 변수
	bool bJustDodgeConsumed = false;

	bool bCanJustDodgeCounter = false;

	FTimerHandle JustDodgeCounterTimerHandle;
};
