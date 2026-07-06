
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Player/Data/SkillDataAsset.h"
#include "SkillComponent.generated.h"

class UAnimInstance;
class UCinematicActionComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API USkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USkillComponent();
	
protected: 
	virtual void BeginPlay() override;
	
	
public:
	// 스킬 발동 요청 (BP/C++에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Skill")
	bool ActivateSkill(USkillDataAsset* SkillData);

	// 현재 스킬/궁극기 컷신이 진행 중인지 확인
	UFUNCTION(BlueprintPure, Category = "Skill")
	bool IsSkillCinematicPlaying() const;

private:
	// 내부에서 시네마틱 요청을 구성하고 호출
	void PlaySkillCinematic(USkillDataAsset* SkillData);

	// 현재 진행 중인 몽타주 정지 (필요시 사용)
	void StopActiveMontages();

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCinematicActionComponent> CinematicActionComp;

	// 현재 발동 중인 스킬 데이터 (중복 방지용)
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Skill", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<const USkillDataAsset> ActiveSkillData;
};