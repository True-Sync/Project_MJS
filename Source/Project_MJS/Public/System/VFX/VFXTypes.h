#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "VFXTypes.generated.h"

class AActor;
class USceneComponent;
class UNiagaraSystem;

// 이펙트를 해시 단위로 관리하기 위한 구조체.
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FVFXHandle
{
	GENERATED_BODY()

public:
	FVFXHandle() = default;
	explicit FVFXHandle(const FGuid& InId) : Id(InId) {}

	UPROPERTY(BlueprintReadOnly, Category = "VFX")
	FGuid Id;

	bool IsValid() const { return Id.IsValid(); }
	void Reset() { Id = FGuid(); }
	bool operator==(const FVFXHandle& Other) const { return Id == Other.Id; }
	bool operator!=(const FVFXHandle& Other) const { return !(*this == Other); }
	
	// TMap 또는 TSet의 Key로 사용하기 위한 Hash 함수 : 해시 값만 같다고 항상 객체가 다 같은건 아니라서.
	friend uint32 GetTypeHash(const FVFXHandle& Handle) { return GetTypeHash(Handle.Id); }
};

// 부착 여부 
UENUM(BlueprintType)
enum class EVFXAttachmentMode : uint8
{
	WorldLocation        UMETA(DisplayName = "World Location"),
	AttachToComponent    UMETA(DisplayName = "Attach To Component"),
	AttachToSocket       UMETA(DisplayName = "Attach To Socket")
};

// VFX 수명 관련. 
UENUM(BlueprintType)
enum class EVFXLifetimePolicy : uint8
{
	// 재생이 끝나면 자동으로 제거되는 일반 효과 
	OneShot UMETA(DisplayName = "One Shot"),

	// 명시적으로 Stop할 때까지 유지되는 효과 
	Persistent UMETA(DisplayName = "Persistent")
};


// 같은 종류의 VFX가 중복 생성되었을시
UENUM(BlueprintType)
enum class EVFXDuplicatePolicy : uint8
{
	// 기존 인스턴스와 관계없이 새 인스턴스 생성 
	AllowMultiple UMETA(DisplayName = "Allow Multiple"),

	// 같은 태그의 인스턴스가 있으면 새 요청 무시 
	IgnoreNew UMETA(DisplayName = "Ignore New"),

	// 같은 태그의 기존 인스턴스를 제거하고 새로 실행 
	ReplaceSameTag UMETA(DisplayName = "Replace Same Tag"),

	// 이 Executor가 관리하는 기존 VFX를 모두 제거하고 실행 
	ReplaceAll UMETA(DisplayName = "Replace All")
};



/*
 VFX를 실행하는 순간에 결정되는 런타임 데이터.
  이번 실행에서 어디에, 누구를 기준으로, 어떤 값과 함께 실행하는가만 전달한다.
  Niagara 에셋, Socket, 수명 정책 같은 정적 설정은 FVFXDefinition이 소유한다.
 */
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FVFXExecuteContext
{
	GENERATED_BODY()

public:
	// VFX 실행을 요청한 액터 
	UPROPERTY(BlueprintReadWrite, Category = "VFX")
	TObjectPtr<AActor> SourceActor = nullptr;

	// VFX의 대상이 되는 액터
	UPROPERTY(BlueprintReadWrite, Category = "VFX")
	TObjectPtr<AActor> TargetActor = nullptr;

	// VFX가 부착될 실제 Scene Component. AttachmentMode가 AttachToComponent 또는 AttachToSocket일 때 사용한다.
	UPROPERTY(BlueprintReadWrite, Category = "VFX|Attachment")
	TObjectPtr<USceneComponent> AttachComponent = nullptr;

	//월드에 생성할 때 사용할 기준 Transform. AttachmentMode가 WorldLocation일 때 사용한다.
	UPROPERTY(BlueprintReadWrite, Category = "VFX|Transform")
	FTransform WorldTransform = FTransform::Identity;

	// 공격, 이동, 투사체 진행 방향 등의 실행 시점 정보 
	UPROPERTY(BlueprintReadWrite, Category = "VFX|Parameters")
	FVector Direction = FVector::ForwardVector;

	// 충돌한 표면의 노멀
	UPROPERTY(BlueprintReadWrite, Category = "VFX|Parameters")
	FVector ImpactNormal = FVector::UpVector;

	/* 현재 실행에만 적용되는 Float Parameter Override.
	 같은 이름의 기본 Parameter가 Definition에 있으면 Context의 값이 우선한다.
	 위에 FloatParameterOverrides 예시:
	 - User.Intensity
	 - User.Speed
	 - User.Damage
	 */
	UPROPERTY(BlueprintReadWrite, Category = "VFX|Parameters")
	TMap<FName, float> FloatParameterOverrides;
	
};


/*
 VFX Profile Data Asset에 저장되는 정적 설정.
 어떤 Niagara를 어떤 정책으로 실행하는가 를 정의한다.
 */
USTRUCT(BlueprintType)
struct PROJECT_MJS_API FVFXDefinition
{
	GENERATED_BODY()

public:
	// 실행할 Niagara System
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	// Niagara 생성 또는 부착 방식
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "VFX|Attachment")
	EVFXAttachmentMode AttachmentMode = EVFXAttachmentMode::WorldLocation;

	// AttachToSocket 모드에서 사용할 Socket 이름. 실제 부착 대상 Component는 ExecuteContext에서 전달한다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Attachment", 
		meta = (EditCondition ="AttachmentMode == EVFXAttachmentMode::AttachToSocket",EditConditionHides))
	FName AttachSocket = NAME_None;

	/*
	  기준 위치 또는 부착 위치에서 적용할 상대 Offset.
	  - WorldLocation : Context.WorldTransform 기준 Offset
	  - Attach 모드: Component 또는 Socket 기준 Offset
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Transform")
	FTransform RelativeOffset = FTransform::Identity;

	// One-shot 또는 Persistent 수명 정책
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Behavior")
	EVFXLifetimePolicy LifetimePolicy = EVFXLifetimePolicy::OneShot;

	// 동일한 태그의 VFX가 중복 실행될 때의 정책 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Behavior")
	EVFXDuplicatePolicy DuplicatePolicy = EVFXDuplicatePolicy::AllowMultiple;
	
	 // Niagara에 적용할 기본 Float User Parameter. ExecuteContext의 FloatParameterOverrides에같은 이름이 있으면 런타임 값이 우선됨.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX|Parameters")
	TMap<FName, float> DefaultFloatParameters;
};