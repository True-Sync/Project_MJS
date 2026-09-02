#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Player/Data/SkillDataAsset.h"
#include "SkillComponent.generated.h"

class UAnimInstance;
class UAnimMontage;
class UCinematicActionComponent;
class USkillDataAsset;
class UStaminaComponent;

UENUM(BlueprintType)
enum class ESkillActivationState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Startup UMETA(DisplayName = "Startup"),
	Cinematic UMETA(DisplayName = "Cinematic"),
	Active UMETA(DisplayName = "Active"),
	Recovery UMETA(DisplayName = "Recovery")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();
	
protected: 
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	// 스킬 발동 요청. BP와 C++에서 모두 호출할 수 있다.
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ActivateSkill(USkillDataAsset* SkillData);

	// AnimNotify 또는 Sequencer Event Track에서 스킬 이벤트를 전달할 때 사용한다.
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void RequestSkillEvent(FName EventName);

	// 현재 스킬 시네마틱이 재생 중인지 확인한다.
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillCinematicPlaying() const;

	UFUNCTION(BlueprintPure, Category = "Skill")
	ESkillActivationState GetSkillState() const { return SkillState; }

private:
	// SkillDataAsset 설정을 기반으로 시네마틱 요청을 구성하고 재생한다.
	void PlaySkillCinematic(USkillDataAsset* SkillData);
	FCinematicPlaybackRequest BuildSkillCinematicRequest(const USkillDataAsset* SkillData) const;
	AActor* ResolveBestSkillTarget(const USkillDataAsset* SkillData) const;
	void AddActorBinding(FCinematicPlaybackRequest& Request, FName BindingTag, AActor* Actor, bool bAllowBindingsFromAsset) const;
	void OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void FinishSkill();

	// 현재 재생 중인 몽타주를 정리한다.
	void StopActiveMontages();

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicActionComponent> CinematicActionComp;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaminaComponent> StaminaComponent;

	// 현재 발동 중인 스킬 데이터. 중복 발동 방지와 이벤트 처리에 사용한다.
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const USkillDataAsset> ActiveSkillData;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	ESkillActivationState SkillState = ESkillActivationState::Idle;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ActiveSkillMontage;
};
