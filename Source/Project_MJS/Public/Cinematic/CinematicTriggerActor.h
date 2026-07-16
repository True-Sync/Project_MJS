#pragma once

#include "CoreMinimal.h"
#include "Cinematic/CinematicTypes.h"
#include "GameFramework/Actor.h"
#include "CinematicTriggerActor.generated.h"

class UBoxComponent;
class ULevelSequence;


/*
 * ACinematicTriggerActor 
 *	레벨에 배치하는 일반 컷신 발동 액터. 
 *	오버랩 조건, 1회 실행 여부, Sequence, AnchorMode, BindingOverrides, Debug 옵션 등을 
 *	에디터 Details 패널에서 설정해서 컷신을 실행한다.
 */

UCLASS()
class PROJECT_MJS_API ACinematicTriggerActor : public AActor
{
	GENERATED_BODY()

public:
	ACinematicTriggerActor();

	
	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	bool ActivateCinematic(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void SetTriggerEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category = "Cinematic")
	void ResetTrigger();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	bool CanTriggerFor(AActor* TargetActor) const;
	void StopTargetActiveMontages(AActor* TargetActor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULevelSequence> Sequence;

	
	// =========== 기본 시퀀서 파라미터 세팅 ===========
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> RequiredActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bTriggerOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bTriggerOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bEnabled = true;

	// 시네마틱 참가자(ICinematicParticipant) 수집 범위. 
	// 기본값은 ExplicitOnly: 불필요한 전역 스캔을 피하고 충돌 위험을 줄인다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	ECinematicParticipantScope ParticipantScope = ECinematicParticipantScope::ExplicitOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bRestoreViewTarget = true;

	// 트리거 컷신에 들어가기 직전 재생 중이던 공격/회피 몽타주를 끊습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bStopTargetMontagesOnTrigger = true;

	// 다른 시네마틱이 이미 재생 중일 때 이 트리거가 기존 컷신을 덮어쓸지 결정합니다.
	// false면 현재 시네마틱이 끝날 때까지 이 트리거는 무시됩니다 (중복/충돌 방지).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true"))
	bool bAllowOverrideWhilePlaying = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BlendOutTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Network", meta = (AllowPrivateAccess = "true"))
	ECinematicNetworkPolicy NetworkPolicy = ECinematicNetworkPolicy::LocalOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|PostAction", meta = (AllowPrivateAccess = "true"))
	FCinematicPostActionConfig PostAction;

	
	// =========== 런타임 바인딩 파라미터 세팅 ===========

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Binding", meta = (AllowPrivateAccess = "true", TitleProperty = "BindingTag"))
	TArray<FCinematicBindingOverride> BindingOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Binding", meta = (AllowPrivateAccess = "true"))
	bool bBindTriggeringActor = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Binding", meta = (AllowPrivateAccess = "true", EditCondition = "bBindTriggeringActor"))
	FName TriggeringActorBindingTag = TEXT("Player");

	
	// =========== 앵커 모드, Rotation 파라미터 세팅 ===========

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	ECinematicAnchorMode AnchorMode = ECinematicAnchorMode::AuthoredWorld;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	ECinematicRotationSource RotationSource = ECinematicRotationSource::AnchorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	bool bUseTriggerActorAsAnchor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	FName AnchorSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	FName TargetSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FTransform ExplicitWorldTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	FRotator ExplicitRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Dynamic Transform", meta = (AllowPrivateAccess = "true"))
	bool bUseYawOnly = true;

	
	// =========== 디버그 표시 파라미터 세팅 ===========

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Debug", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugAnchor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "0.0", EditCondition = "bDrawDebugAnchor"))
	float DebugDrawDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic|Debug", meta = (AllowPrivateAccess = "true", ClampMin = "1.0", EditCondition = "bDrawDebugAnchor"))
	float DebugDrawScale = 120.0f;

	// =========== 상태 플래그 ===========
	UPROPERTY(Transient)
	bool bHasTriggered = false;
};
