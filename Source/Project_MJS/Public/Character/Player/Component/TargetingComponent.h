#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/Player/Component/TargetingTypes.h"
#include "TargetingComponent.generated.h"

class AEnemyCharacter;
class APlayerController;
class USphereComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTargetingDisplayUpdated, bool, const TArray<FTargetingHUDMarkerData>&);
DECLARE_MULTICAST_DELEGATE(FOnTargetingDisplayCleared);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHardTargetChanged, AActor*);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECT_MJS_API UTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTargetingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void RequestHardTarget();
	void BeginRangedHardTargetAim();
	void CompleteRangedHardTargetAim();
	void CancelRangedHardTargetAim();
	void ClearHardTarget();

	AActor* GetSoftTarget() const { return AutoTarget.Get(); }
	AActor* GetAutoTarget() const { return AutoTarget.Get(); }
	AActor* GetHardTarget() const { return HardTarget.Get(); }
	AActor* GetBestAttackTarget() const;
	bool HasAnyTargetCandidate() const { return CandidateTargets.Num() > 0; }

	// 타겟팅 HUD에 크로스헤어/마커 표시 상태를 전달하는 델리게이트
	FOnTargetingDisplayUpdated OnTargetingDisplayUpdated;

	// 타겟팅 컴포넌트가 종료되거나 표시할 UI가 초기화될 때 호출
	FOnTargetingDisplayCleared OnTargetingDisplayCleared;

	// 하드 타겟이 변경될 때 카메라 포커스 쪽으로 알려주는 델리게이트
	FOnHardTargetChanged OnHardTargetChanged;

protected:
	UFUNCTION()
	void OnTargetRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTargetRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void InitializeTargetRange();
	void UpdateTargeting();
	void UpdateTargetingTimerState();
	void PublishTargetingDisplay();
	void DrawDebugTargetingRange() const;
	void UpdateComponentTickState();
	void RemoveInvalidCandidates();
	void SetAutoTarget(AActor* NewTarget);
	void SetHardTarget(AActor* NewTarget, EHardTargetingMode NewMode);
	void UpdateHardTargetVisibility(float DeltaTime);

	AActor* ChooseAutoTarget() const;
	AActor* ChooseNextHardTarget() const;
	AActor* TraceRangedHardTarget() const;
	APlayerController* GetOwningPlayerController() const;
	bool IsValidCandidate(AActor* Candidate) const;
	bool IsInsideAutoTargetRange(AActor* Candidate) const;
	bool IsInsideHardTargetRange(AActor* Candidate) const;
	bool HasAutoTargetCandidate() const;
	bool ShouldShowCrosshair() const;
	bool ShouldDisplayMarkerForTarget(AActor* Target) const;
	bool IsTargetVisibleOnScreen(AActor* Target) const;
	float GetCrosshairDistance(const AActor* Candidate) const;
	FVector GetTargetWorldLocation(const AActor* Target) const;

	// ===== 타겟팅 범위 =====
	UPROPERTY(EditAnywhere, Category = "Targeting|Range")
	float AutoTargetRadius = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Range")
	float HardTargetRadius = 1800.0f;

	// ===== 하드 타겟팅 =====
	// 하드 타겟이 화면 밖으로 나간 뒤 자동 해제되기까지 기다리는 시간 (이 부분은 전투 이탈 7초 후 해제로 변경될 수 있음)
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard")
	float HardTargetOutOfSightGraceTime = 0.5f;

	// 원거리 하드 타겟팅 조준 트레이스가 사용할 콜리전 채널
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard")
	TEnumAsByte<ECollisionChannel> RangedHardTargetTraceChannel = ECC_Visibility;

	// true면 라인 트레이스 대신 구체 스윕으로 판정
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard")
	bool bUseRangedHardTargetSphereTrace = true;

	// 원거리 하드 타겟팅 구체 스윕의 반지름
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard", meta = (EditCondition = "bUseRangedHardTargetSphereTrace", ClampMin = "1.0"))
	float RangedHardTargetTraceRadius = 30.0f;

	// 카메라가 플레이어 뒤에 있는 거리까지 감안해서 트레이스를 조금 더 길게 쏘기 위한 추가 거리
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard", meta = (ClampMin = "0.0"))
	float RangedHardTargetTraceExtraLength = 300.0f;

	// false = 캡슐, 박스 위주 / true = StaticMesh/SkeletalMesh 위주 콜리전 검사
	UPROPERTY(EditAnywhere, Category = "Targeting|Hard")
	bool bUseComplexCollisionForRangedTrace = false;

	// ===== 소프트 타겟팅 =====
	// 후보가 있을 때 소프트 타겟을 다시 고르는 주기
	UPROPERTY(EditAnywhere, Category = "Targeting|Auto")
	float AutoTargetUpdateInterval = 0.08f;

	// 화면 중앙 크로스헤어에 이 픽셀 거리 안으로 들어온 대상은 우선 소프트 타겟 선정
	UPROPERTY(EditAnywhere, Category = "Targeting|Auto")
	float DirectCrosshairPixelRadius = 80.0f;

	// 크로스헤어와 거리 점수를 섞어 소프트 타겟을 고를 때 사용하는 화면 기준 반경
	UPROPERTY(EditAnywhere, Category = "Targeting|Auto")
	float ScreenCandidatePixelRadius = 320.0f;

	// ===== 디버그 =====
	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bDrawDebugTargetingRange = false;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugTargetingRange"))
	FColor AutoTargetDebugColor = FColor::Cyan;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugTargetingRange"))
	FColor HardTargetDebugColor = FColor::Yellow;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bDrawDebugRangedHardTargetTrace = false;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug")
	bool bLogRangedHardTargetTrace = false;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugRangedHardTargetTrace", ClampMin = "0.0"))
	float RangedHardTargetTraceDebugDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugRangedHardTargetTrace"))
	FColor RangedHardTargetTraceHitColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugRangedHardTargetTrace"))
	FColor RangedHardTargetTraceMissColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "Targeting|Debug", meta = (EditCondition = "bDrawDebugRangedHardTargetTrace"))
	FColor RangedHardTargetTraceRejectedColor = FColor::Orange;

	// ===== 런타임 상태 =====
	// 플레이어에게 붙는 타겟 후보 감지용 구체 콜리전
	UPROPERTY(Transient)
	TObjectPtr<USphereComponent> TargetRangeSphere;

	// 하드 타겟팅 범위 안에 들어온 적 후보 목록
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> CandidateTargets;

	// 현재 소프트 타겟팅으로 선택된 대상
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> AutoTarget;

	// 현재 하드 타겟팅으로 고정된 대상
	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> HardTarget;

	// 현재 하드 타겟팅이 일반/원거리 중 어떤 방식으로 잡혔는지 기록
	UPROPERTY(Transient)
	EHardTargetingMode HardTargetingMode = EHardTargetingMode::None;

	// 원거리 하드 타겟팅 입력을 누르고 조준 중인지 여부
	UPROPERTY(Transient)
	bool bRangedHardTargetAiming = false;

	// 하드 타겟이 화면 밖에 머문 누적 시간
	UPROPERTY(Transient)
	float HardTargetOutOfSightTime = 0.0f;

	// 후보가 있을 때만 소프트 타겟팅을 갱신하기 위한 TimerHandle
	FTimerHandle TargetingUpdateTimerHandle;
};
