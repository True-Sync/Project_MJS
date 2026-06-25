#pragma once

#include "CoreMinimal.h"
#include "TargetingTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EHardTargetingMode : uint8
{
	// 하드 타겟팅이 걸려있지 않은 상태
	None,

	// 소프트 타겟팅 범위 안의 대상을 우클릭으로 하드 타겟팅한 상태
	Auto,

	// 원거리 하드 타겟팅 입력으로 직접 조준해서 잡은 상태
	Ranged
};

UENUM(BlueprintType)
enum class ETargetingHUDMarkerType : uint8
{
	// 타겟팅 가능하지만 아직 선택되지는 않은 대상
	Targetable,

	// 현재 소프트 타겟팅으로 잡힌 대상
	AutoTarget,

	// 현재 하드 타겟팅으로 고정된 대상
	HardTarget
};

USTRUCT(BlueprintType)
struct FTargetingHUDMarkerData
{
	GENERATED_BODY()

	// UI 마커가 따라갈 실제 액터
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|HUD")
	TObjectPtr<AActor> TargetActor = nullptr;

	// 마커를 띄울 월드 위치. 보통 적의 몸통 중앙 TargetPoint를 사용.
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|HUD")
	FVector WorldLocation = FVector::ZeroVector;

	// HUD에 배치할 화면 좌표
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|HUD")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	// 어떤 모양의 타겟팅 UI를 띄울지 결정하는 타입
	UPROPERTY(BlueprintReadOnly, Category = "Targeting|HUD")
	ETargetingHUDMarkerType MarkerType = ETargetingHUDMarkerType::Targetable;
};
