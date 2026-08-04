#include "System/VFX/VFXExcutorComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "SNegativeActionButton.h"
#include "System/VFX/Data/CharacterVFXProfile.h"

UVFXExcutorComponent::UVFXExcutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVFXExcutorComponent::ExecuteVFX(const FGameplayTag& Tag, const FVFXExecuteContext& Context)
{
	UE_LOG(LogTemp, Warning, TEXT("[VFX] ExecuteVFX 시작 | Tag: %s"),*Tag.ToString());
		
	if (!Tag.IsValid() || !CharacterVfxProfile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: 유효하지 않은 GameplayTag 또는 CharacterVfxProfile"));
		return;
	}
	
	const FVFXDefinition* Definition = CharacterVfxProfile->FindDefinition(Tag);
	
	if (!Definition)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: Profile에서 Definition을 찾지 못함 | Tag: %s"),*Tag.ToString());
		return;
	}
	
	// 4. 현재는 WorldLocation만 테스트
	if (Definition->AttachmentMode != EVFXAttachmentMode::WorldLocation)
	{
		UE_LOG(LogTemp,Warning,TEXT("[VFX] 실행 중단: 현재 테스트는 WorldLocation만 지원 | 현재 Mode: %d"), 
			static_cast<int32>(Definition->AttachmentMode));
		return;
	}
	
	if (Definition->NiagaraSystem.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: NiagaraSystem이 지정되지 않음 | Tag: %s"),*Tag.ToString());
		return;
	}
	
	
	UNiagaraSystem* NiagaraSystem = Definition->NiagaraSystem.LoadSynchronous();

	if (!IsValid(NiagaraSystem))
	{
		UE_LOG(LogTemp,Error,TEXT("[VFX] 실행 실패: NiagaraSystem 로드 실패 | Asset: %s"),*Definition->NiagaraSystem.ToSoftObjectPath().ToString());
		return;
	}

	// 6. Niagara 생성
	UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			NiagaraSystem,
			Context.WorldTransform.GetLocation(),
			Context.WorldTransform.Rotator(),
			Context.WorldTransform.GetScale3D(),
			true,
			true,
			ENCPoolMethod::None,
			true
		);

	if (!IsValid(SpawnedComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("[VFX] 실행 실패: Niagara Component 생성 실패 | Tag: %s"), *Tag.ToString());
		return;
	}

	UE_LOG(LogTemp,Log,TEXT("[VFX] 실행 성공 | Tag: %s | Niagara: %s | Location: %s"),
		*Tag.ToString(),*GetNameSafe(NiagaraSystem),*Context.WorldTransform.GetLocation().ToString()
	);
}



