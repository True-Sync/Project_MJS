#pragma once

#include "CoreMinimal.h"
#include "Logging/LogMacros.h"
#include "UObject/ObjectPtr.h"
#include "CinematicTypes.generated.h"

// 전용 로그 카테고리: 시네마틱 시스템 전체에서 사용
DECLARE_LOG_CATEGORY_EXTERN(LogCinematicSystem, Log, All);

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
 2.5 ECinematicParticipantScope
	시네마틱 참가자(ICinematicParticipant)를 어떻게 수집할지 결정한다. 
	기존 bAffectAllParticipants 대신 이 옵션을 사용해서 성능과 안정성을 높인다.
 */
UENUM(BlueprintType)
enum class ECinematicParticipantScope : uint8
{
	// InstigatorActor, SubjectActor, AdditionalParticipants에 명시된 대상만 수집.
	ExplicitOnly UMETA(DisplayName = "Explicit Only"),

	// 월드 내 모든 ICinematicParticipant를 스캔해서 수집 (기존 bAffectAllParticipants == true 동작).
	// 대규모 맵에서 오버헤드가 크므로 필요할 때만 사용 권장.
	AllInWorld UMETA(DisplayName = "All In World")
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
 3.5 ECinematicPreset (선택적 가이드용)
	FCinematicPlaybackRequest의 여러 옵션을 어떤 조합으로 설정해야 하는지 안내하는 프리셋 개념. 
	별도 필드로 강제하지 않고, 사용 예시로만 제공한다. 실제 값은 아래 각 항목에서 명시한다.
 */
UENUM(BlueprintType)
enum class ECinematicPreset : uint8
{
	// 스킬/궁극기 컷신: 플레이어 중심, 짧은 시간, 기존 시네마틱과 충돌하면 무시하는 것이 안전.
	SkillCutscene UMETA(DisplayName = "Skill Cutscene"),

	// 로컬 이벤트용: 특정 액터만 영향 받고, 다른 컷신과 겹칠 때 충돌 방지 우선.
	LocalEvent UMETA(DisplayName = "Local Event"),

	// 전역 컷신/스토리 시네마틱: 전체 참가자 수집, 기존 컷신 중단 허용, 카메라 복구 필수.
	GlobalCinematic UMETA(DisplayName = "Global Cinematic")
};


/* ====================================================================================================
 3.8 FCinematicPostActionConfig
	시네마틱 종료 후 수행할 액션 설정. 처음에는 레벨 로딩만 지원하고, 추후 필요하면 같은 구조체에 옵션 추가.
*/
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FCinematicPostActionConfig
{
	GENERATED_BODY()

	// 시퀀스 종료 후 지정된 레벨로 이동할지 결정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction")
	bool bLoadLevelOnFinish = false;

	// 이동할 레벨 경로. 예: "/Game/Maps/NextChapter" 또는 맵 에셋 레퍼런스 이름.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction", meta = (EditCondition = "bLoadLevelOnFinish || bLoadLevelDuringPlayback"))
	FName LevelName;

	// true면 현재 월드에 등록된 스트리밍 레벨을 비동기로 로드/표시하고, false면 OpenLevel로 맵을 전환.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction", meta = (EditCondition = "bLoadLevelOnFinish || bLoadLevelDuringPlayback"))
	bool bAsyncLoad = true;

	// 시퀀스 종료 후 레벨 로딩 전 지연 시간. 컷신 종료 연출과 로딩 시작 사이 여백용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction", meta = (ClampMin = "0.0", EditCondition = "bLoadLevelOnFinish"))
	float DelayBeforeLoad = 0.0f;

	// 시퀀스가 아직 재생 중일 때도 레벨 로딩을 시작할지 결정.
	// true면 컷신이 끝나기 전에 다음 스트리밍 레벨의 로드/표시를 요청할 수 있음.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction|During Playback")
	bool bLoadLevelDuringPlayback = false;

	// 시퀀스 재생 시작 후 몇 초에 레벨 로딩을 트리거할지 결정.
	// 0.0f면 컷신 시작과 동시에 로딩 시작, 양수면 해당 시간 지점에 로딩 시작.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction|During Playback",
		meta = (ClampMin = "0.0", EditCondition = "bLoadLevelDuringPlayback"))
	float LoadLevelTriggerTime = 0.0f;

	// 레벨 로딩 직전에 현재 시네마틱을 강제로 정지할지 결정.
	// true면 로딩 전에 컷신을 멈추고 정리, false면 컷신은 그대로 재생하면서 로딩만 병렬로 진행.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction|During Playback",
		meta = (EditCondition = "bLoadLevelDuringPlayback"))
	bool bStopCinematicBeforeLoad = false;
};

/* ====================================================================================================
4. FCinematicPlaybackRequest (좀 뭔가 많다.)
	시네마틱 재생을 시작할 때 요청자에서 DirectorSubsystem으로 전달하는 데이터. 
	스킬, 궁극기, 일반 컷신 트리거가 모두 이 구조체 하나로 재생을 요청한다.

	[ECinematicPreset별 권장 설정 예시]
	- SkillCutscene:
	  - ParticipantScope = ExplicitOnly (또는 필요한 경우 AllInWorld)
	  - bStopPreviousCinematic = false (충돌 시 스킬 컷신은 실패하는 것이 안전)
	  - AnchorMode = InstigatorToSubject 또는 SubjectActor 등 동적 원점 사용 권장
	- LocalEvent:
	  - ParticipantScope = ExplicitOnly
	  - bStopPreviousCinematic = false
	- GlobalCinematic:
	  - ParticipantScope = AllInWorld
	  - bStopPreviousCinematic = true (스토리 컷신은 우선)
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

	// 4.7 시네마틱 참가자(ICinematicParticipant) 수집 범위. 
	// 기본값은 ExplicitOnly: 불필요한 전역 스캔을 피하고 충돌 위험을 줄인다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	ECinematicParticipantScope ParticipantScope = ECinematicParticipantScope::ExplicitOnly;

	// 4.8 시네마틱 종료 뒤 이전 카메라 ViewTarget으로 복구할지 결정.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bRestoreViewTarget = true;

	// 4.9 이미 다른 시네마틱이 재생 중일 때 기존 재생을 중단하고 새 요청을 시작할지 결정.
	// 기본값은 false로 변경: 의도치 않은 컷신 덮어쓰기를 방지한다. 전역 스토리 컷신 등에서는 true를 명시.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic")
	bool bStopPreviousCinematic = false;

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

	// 4.24 시네마틱 종료 후 수행할 액션 설정. 현재는 레벨 로딩만 지원하며 추후 확장 가능.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic|PostAction")
	FCinematicPostActionConfig PostAction;
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

	// 이번 재생의 참가자 수집 범위 (ExplicitOnly / AllInWorld).
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic")
	ECinematicParticipantScope ParticipantScope = ECinematicParticipantScope::ExplicitOnly;

	// 이번 재생에서 사용한 동적 원점 계산 방식.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	ECinematicAnchorMode AnchorMode = ECinematicAnchorMode::AuthoredWorld;

	// 이번 재생에서 사용한 회전 기준.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	ECinematicRotationSource RotationSource = ECinematicRotationSource::AnchorTransform;

	// Director가 계산한 최종 동적 시퀀스 원점. 스킬 판정, 추가 VFX, 디버그 표시가 같은 기준을 재사용할 때 참고.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	FTransform AnchorWorldTransform = FTransform::Identity;

	// 이번 재생에서 동적 원점이 실제로 적용됐는지 표시. false면 Sequence 에셋에 저장된 월드 위치 그대로 재생된 것.
	UPROPERTY(BlueprintReadOnly, Category = "Cinematic|Dynamic Transform")
	bool bAppliedDynamicTransform = false;
};
