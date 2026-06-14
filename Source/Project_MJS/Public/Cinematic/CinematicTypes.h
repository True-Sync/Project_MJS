#pragma once

#include "CoreMinimal.h"
#include "CinematicTypes.generated.h"

class AActor;
class ALevelSequenceActor;
class APlayerController;
class ULevelSequence;

/* ====================================================================================================
 1. ECinematicAnchorMode
	시퀀스가 월드 안에서 어디를 원점으로 삼아 재생될지 결정한다.
	스킬 컷신은 보통 플레이어 위치나 플레이어-타겟 방향을 쓰고,
	일반 컷신은 에셋에 저장된 월드 위치를 그대로 쓰거나 트리거 액터 위치를 앵커로 쓴다.
 */
UENUM(BlueprintType)
enum class ECinematicAnchorMode : uint8
{
	// Level Sequence 에셋에 저장된 위치/회전을 그대로 사용.
	AuthoredWorld UMETA(DisplayName = "Authored World"),

	// InstigatorActor의 위치/회전을 동적 원점으로 사용.
	InstigatorActor UMETA(DisplayName = "Instigator Actor"),

	// SubjectActor의 위치/회전을 동적 원점으로 사용.
	SubjectActor UMETA(DisplayName = "Subject Actor"),

	// InstigatorActor 위치에서 SubjectActor를 바라보는 방향을 동적 원점으로 사용.
	InstigatorToSubject UMETA(DisplayName = "Instigator To Subject"),

	// 요청자가 넘긴 ExplicitWorldTransform을 동적 원점으로 사용.
	ExplicitTransform UMETA(DisplayName = "Explicit Transform")
};

/* ====================================================================================================
 2. ECinematicRotationSource
	앵커 위치가 정해진 뒤, 최종 시퀀스 회전을 어디서 가져올지 결정한다. 
	위치는 플레이어 기준이지만 방향은 카메라 기준이어야 하는 스킬을 위해 분리했다. 
 */
UENUM(BlueprintType)
enum class ECinematicRotationSource : uint8
{
	// AnchorMode가 계산한 회전을 그대로 사용.
	AnchorTransform UMETA(DisplayName = "Anchor Transform"),

	// InstigatorActor의 월드 회전을 사용.
	InstigatorActor UMETA(DisplayName = "Instigator Actor"),

	// SubjectActor의 월드 회전을 사용.
	SubjectActor UMETA(DisplayName = "Subject Actor"),

	// PlayerController의 ControlRotation을 사용.
	PlayerControlRotation UMETA(DisplayName = "Player Control Rotation"),

	// PlayerCameraManager의 카메라 회전을 사용.
	PlayerCameraRotation UMETA(DisplayName = "Player Camera Rotation"),

	// 요청자가 넘긴 ExplicitRotation을 사용.
	ExplicitRotation UMETA(DisplayName = "Explicit Rotation")
};

// ====================================================================================================
// Net관련 유틸인데, 이거는 일단 졸작까지는 LocalOnly로 작동시킵시다.(멀티 게임 아니자냐 ㅡ.ㅡ)
UENUM(BlueprintType)
enum class ECinematicNetworkPolicy : uint8
{
	LocalOnly UMETA(DisplayName = "Local Only"),
	AuthorityOnly UMETA(DisplayName = "Authority Only"),
	AnyNetMode UMETA(DisplayName = "Any Net Mode")
};


/* ====================================================================================================
 3. FCinematicBindingOverride 
	 Sequencer Binding Tag 기반 런타임 액터 교체
	 트리거 편의 옵션: 오버랩한 액터를 기본 Player 태그에 자동 바인딩하는 bBindTriggeringActor
*/
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FCinematicBindingOverride
{
	GENERATED_BODY()

	// Level Sequence 안에서 태그로 찾을 바인딩 이름. Sequencer에서 Actor Binding 우클릭 -> Tags로 지정한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Binding")
	FName BindingTag = NAME_None;

	// BindingTag에 연결할 런타임 액터 목록. 예: Player, Target, Weapon.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Binding")
	TArray<TObjectPtr<AActor>> Actors;

	// true면 에셋에 원래 들어 있던 바인딩을 유지하면서 런타임 액터를 추가. false면 런타임 액터로 교체.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Binding")
	bool bAllowBindingsFromAsset = false;
};

/* ====================================================================================================
4. FCinematicPlaybackRequest (좀 뭔가 많다.)
	시네마틱 재생을 시작할 때 요청자에서 DirectorSubsystem으로 전달하는 데이터. 
	스킬, 궁극기, 일반 컷신 트리거가 모두 이 구조체 하나로 재생을 요청한다.
*/
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FCinematicPlaybackRequest
{
	GENERATED_BODY()

	// 4.1 실제로 재생할 Level Sequence 에셋. 비어 있으면 재생 요청은 실패.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TObjectPtr<ULevelSequence> Sequence = nullptr;

	// 4.2 시네마틱을 발동시킨 액터. 스킬이면 보통 플레이어, 트리거 컷신이면 트리거에 들어온 액터.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	// 4.3 시네마틱의 중심 대상 액터. 카메라가 따라갈 캐릭터, 공격 대상, 이벤트 대상 등에 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TObjectPtr<AActor> SubjectActor = nullptr;

	// 4.4 입력 잠금과 ViewTarget 복구에 사용할 플레이어 컨트롤러. 비워두면 Director가 Instigator/Subject에서 자동으로 찾고, 실패하면 월드의 첫 번째 컨트롤러를 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// 4.5 Instigator/Subject 외에 시네마틱 참가자로 직접 포함할 액터 목록.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	TArray<TObjectPtr<AActor>> AdditionalParticipants;

	// 4.6 Sequencer의 Binding Tag를 런타임 액터로 교체하는 목록. Player/Target/Weapon 같은 태그를 실제 액터로 연결할 때 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Binding", meta = (TitleProperty = "BindingTag"))
	TArray<FCinematicBindingOverride> BindingOverrides;

	// 4.7 true면 월드 안의 모든 CinematicParticipant를 수집. false면 Instigator/Subject/AdditionalParticipants에 명시된 대상만 반응.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bAffectAllParticipants = true;

	// 4.8 시네마틱 종료 뒤 이전 카메라 ViewTarget으로 복구할지 결정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bRestoreViewTarget = true;

	// 4.9 이미 다른 시네마틱이 재생 중일 때 기존 재생을 중단하고 새 요청을 시작할지 결정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bStopPreviousCinematic = true;

	// 4.10 네트워크 환경에서 이 시네마틱을 어느 쪽에서 재생할지 결정. 기본값은 로컬 전용으로, 전용 서버에서는 재생하지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Network")
	ECinematicNetworkPolicy NetworkPolicy = ECinematicNetworkPolicy::LocalOnly;

	// 4.11 시네마틱 종료 뒤 기존 ViewTarget으로 복구할 때 사용할 카메라 블렌드 시간.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic", meta = (ClampMin = "0.0"))
	float BlendOutTime = 0.15f;

	// 4.12 동적 시퀀스 원점을 계산하는 방식. AuthoredWorld면 Level Sequence 에셋에 저장된 위치를 그대로 쓰고, 나머지는 런타임에 원점을 계산.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	ECinematicAnchorMode AnchorMode = ECinematicAnchorMode::AuthoredWorld;

	// 4.13 최종 시퀀스 회전을 어디서 가져올지 결정. 예시로는 위치는 플레이어 기준, 방향은 카메라 기준인 스킬은 PlayerCameraRotation을 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	ECinematicRotationSource RotationSource = ECinematicRotationSource::AnchorTransform;

	// 4.14 AnchorMode의 기본 대상 대신 사용할 명시적 앵커 액터. 비워두면 InstigatorActor 또는 SubjectActor를 모드에 맞춰 자동으로 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	TObjectPtr<AActor> AnchorActor = nullptr;

	// 4.15 앵커 액터 안의 특정 소켓/본을 원점으로 사용할 때 지정. 비어 있거나 소켓을 찾지 못하면 액터의 월드 트랜스폼을 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	FName AnchorSocketName = NAME_None;

	// 4.16 InstigatorToSubject 모드에서 바라볼 대상 소켓/본. 비어 있거나 소켓을 찾지 못하면 SubjectActor의 액터 위치를 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	FName TargetSocketName = NAME_None;

	// 4.17 계산된 앵커 기준으로 추가 적용할 로컬 오프셋.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform", meta = (MakeEditWidget = "true"))
	FTransform RelativeTransform = FTransform::Identity;

	// 4.18 AnchorMode가 ExplicitTransform일 때 사용할 완전한 월드 트랜스폼.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform", meta = (MakeEditWidget = "true"))
	FTransform ExplicitWorldTransform = FTransform::Identity;

	// 4.19 RotationSource가 ExplicitRotation일 때 사용할 명시적 회전값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	FRotator ExplicitRotation = FRotator::ZeroRotator;

	// 4.20 true면 회전에서 Yaw만 사용하고 Pitch/Roll은 0으로 만듬. 캐릭터 액션 게임의 지상 스킬 컷신은 보통 이 값을 true로 두면 됨.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Dynamic Transform")
	bool bUseYawOnly = true;

	// 4.21 true면 계산된 동적 앵커 위치에 좌표축과 구체를 그린다. 시퀀스가 엉뚱한 위치에서 재생될 때 원인 확인용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Debug")
	bool bDrawDebugAnchor = false;

	// 4.22 앵커 디버그 표시가 유지되는 시간.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Debug", meta = (ClampMin = "0.0", EditCondition = "bDrawDebugAnchor"))
	float DebugDrawDuration = 3.0f;

	// 4.23 앵커 디버그 좌표축 크기.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|Debug", meta = (ClampMin = "1.0", EditCondition = "bDrawDebugAnchor"))
	float DebugDrawScale = 120.0f;
};

/* ====================================================================================================
 5. FCinematicPlaybackContext
	시네마틱이 실제로 재생되는 동안 참가자에게 전달되는 읽기 전용 상태 정보. 
	참가자 컴포넌트나 인터페이스 구현체는 이 정보를 보고 입력, 이동, Tick, 애니메이션 등을 멈추거나 복구한다.
*/
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FCinematicPlaybackContext
{
	GENERATED_BODY()

	// 현재 재생 중인 Level Sequence 에셋.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	TObjectPtr<ULevelSequence> Sequence = nullptr;

	// 현재 시네마틱을 발동시킨 액터.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	// 현재 시네마틱의 중심 대상 액터.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	TObjectPtr<AActor> SubjectActor = nullptr;

	// 현재 시네마틱에서 입력 잠금과 ViewTarget 복구에 사용하는 플레이어 컨트롤러.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	TObjectPtr<APlayerController> PlayerController = nullptr;

	// DirectorSubsystem이 런타임에 생성한 LevelSequenceActor. 시퀀스가 끝나면 Director가 정리.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	TObjectPtr<ALevelSequenceActor> SequenceActor = nullptr;

	// 이번 재생이 전체 참가자에게 영향을 주는지, 명시된 참가자에게만 영향을 주는지 표시.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	bool bAffectAllParticipants = false;

	// Director가 계산한 최종 동적 시퀀스 원점. 스킬 판정, 추가 VFX, 디버그 표시가 같은 기준을 재사용할 때 참고.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	FTransform AnchorWorldTransform = FTransform::Identity;

	// 이번 재생에서 동적 원점이 실제로 적용됐는지 표시. false면 Sequence 에셋에 저장된 월드 위치 그대로 재생된 것.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	bool bAppliedDynamicTransform = false;
};
