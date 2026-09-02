#include "System/VFX/VFXExecutorComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "System/VFX/Data/CharacterVFXProfile.h"

UVFXExecutorComponent::UVFXExecutorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FVFXHandle UVFXExecutorComponent::ExecuteVFX(const FGameplayTag& Tag, const FVFXExecuteContext& Context)
{
	UE_LOG(LogTemp, Warning, TEXT("[VFX] ExecuteVFX 시작 | Tag: %s"),*Tag.ToString());
	
	const FVFXHandle InvalidHandle; // return 전용 핸들 생성
	
	if (!Tag.IsValid() || !CharacterVfxProfile)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: 유효하지 않은 GameplayTag 또는 CharacterVfxProfile"));
		return InvalidHandle;
	}
	
	const FVFXDefinition* Definition = CharacterVfxProfile->FindDefinition(Tag);
	
	if (!Definition)
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: Profile에서 Definition을 찾지 못함 | Tag: %s"),*Tag.ToString());
		return InvalidHandle;
	}
	
	if (Definition->NiagaraSystem.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패: NiagaraSystem이 지정되지 않음 | Tag: %s"),*Tag.ToString());
		return InvalidHandle;
	}
	
	
	UNiagaraComponent* SpawnedComponent = SpawnNiagara(Tag, *Definition, Context);
	if (!IsValid(SpawnedComponent))
		return InvalidHandle;
	
	// 파라미터 적용
	ApplyFloatParameters(SpawnedComponent, *Definition, Context);
	
	if (Definition->LifetimePolicy == EVFXLifetimePolicy::Persistent)
	{
		SpawnedComponent->OnSystemFinished.AddDynamic(this, &UVFXExecutorComponent::OnNiagaraFinished);
		
		const FVFXHandle NewHandle(FGuid::NewGuid());
		FVFXInstanceData InstanceData;
		InstanceData.NiagaraComponent = SpawnedComponent;
		InstanceData.Tag = Tag;

		ActiveInstances.Add(NewHandle, InstanceData);
		SpawnedComponent->Activate(true);
		
		UE_LOG(LogTemp, Log, TEXT("[VFX] 실행 성공 | Tag: %s | Component: %s"),*Tag.ToString(), *GetNameSafe(SpawnedComponent));
		return NewHandle;
	}
	
	// OneShot은 핸들 발급 제외.(저장 안하고 바로 파괴임.) -> 잘못하다가 Map에 저장되어서 댕글링 포인터 발생.
	if (Definition->LifetimePolicy == EVFXLifetimePolicy::OneShot)
	{
		SpawnedComponent->Activate(true);
		UE_LOG(LogTemp,Log,TEXT("[VFX] OneShot 실행 성공 | Tag: %s | Component: %s"),*Tag.ToString(),*GetNameSafe(SpawnedComponent));
		return InvalidHandle;
	}
	
	return InvalidHandle;
}


UNiagaraComponent* UVFXExecutorComponent::SpawnNiagara(const FGameplayTag& Tag, const FVFXDefinition& ExecuteDefinition, const FVFXExecuteContext& Context)
{
	const bool bAutoDestroy = ExecuteDefinition.LifetimePolicy == EVFXLifetimePolicy::OneShot;
	
	// Niagara 시스템 및 컴포넌트 생성
	UNiagaraSystem* NiagaraSystem = ExecuteDefinition.NiagaraSystem.LoadSynchronous();
	if (!IsValid(NiagaraSystem))
	{
		UE_LOG(LogTemp,Error,TEXT("[VFX] 실행 실패: NiagaraSystem 로드 실패 | Asset: %s"), *ExecuteDefinition.NiagaraSystem.ToSoftObjectPath().ToString());
		return nullptr;
	}
	
	UNiagaraComponent* SpawnedComponent = nullptr;
	switch (ExecuteDefinition.AttachmentMode)
	{
	case EVFXAttachmentMode::WorldLocation:
		{
			const FTransform SpawnTransform = ExecuteDefinition.RelativeOffset * Context.WorldTransform;

				
			SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
				NiagaraSystem,
				SpawnTransform.GetLocation(),
				SpawnTransform.Rotator(),
				SpawnTransform.GetScale3D(),
				bAutoDestroy,                  
				false,                  
				ENCPoolMethod::None,
				true                   
			);

			break;
		}
	case EVFXAttachmentMode::AttachToComponent:
	case EVFXAttachmentMode::AttachToSocket: //Socket 부착시 반드시 AttachComponent 필요. 
		{
			if (!IsValid(Context.AttachComponent))
			{
				UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패 | AttachComponent가 없음 | Tag: %s"),*Tag.ToString());
				return nullptr;
			}
			
			FName AttachPointName = NAME_None;
			
			//Socket 모드 일때만 Definition의 Socket 사용. 
			if (ExecuteDefinition.AttachmentMode == EVFXAttachmentMode::AttachToSocket)
			{
				if (ExecuteDefinition.AttachSocket.IsNone())
				{
					UE_LOG(LogTemp, Warning, TEXT("[VFX] 실행 실패 | AttachSocket이 지정되지 않음. | Tag : %s"), *Tag.ToString());
					return nullptr;
				}
				AttachPointName = ExecuteDefinition.AttachSocket;
			}
			SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				NiagaraSystem,
				Context.AttachComponent,
				AttachPointName,
				ExecuteDefinition.RelativeOffset.GetLocation(),
			ExecuteDefinition.RelativeOffset.Rotator(),
				ExecuteDefinition.RelativeOffset.GetScale3D(),
				EAttachLocation::KeepRelativeOffset,
				bAutoDestroy,                  
				ENCPoolMethod::None,
				false,                 
				true                   
			);
			break;
		}
	default:
		{
			UE_LOG(LogTemp, Error, TEXT("[VFX] 지원하지 않는 AttachmentMode | Tag: %s"), *Tag.ToString());
			return nullptr;
		}
	}		
	
	if (!IsValid(SpawnedComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("[VFX] 실행 실패: Niagara Component 생성 실패 | Tag: %s"), *Tag.ToString());
		return nullptr;
	}
	
	return SpawnedComponent;
}

void UVFXExecutorComponent::ApplyFloatParameters(UNiagaraComponent* NiagaraComponent, const FVFXDefinition& Definition,
	const FVFXExecuteContext& Context)
{
	if (!IsValid(NiagaraComponent))
		return;
	
	//Profile 기본 값 그대로
	for (const TPair<FName, float>& Parameter : Definition.DefaultFloatParameters)
	{
		NiagaraComponent->SetVariableFloat(Parameter.Key, Parameter.Value);
	}
	
	//Override -> 나중에 적용하니까 기본값보다는 더 우선시함.
	for (const TPair<FName, float>& Parameter : Context.FloatParameterOverrides)
	{
		NiagaraComponent->SetVariableFloat(Parameter.Key, Parameter.Value);
	}
}


// ================ Stop 판정 ===================


bool UVFXExecutorComponent::StopVFX(const FVFXHandle& Handle)
{
	if (!Handle.IsValid())
		return false;
	
	FVFXInstanceData* Instance = ActiveInstances.Find(Handle);
	if (!Instance || !IsValid(Instance->NiagaraComponent))
		return false;
	
	Instance->NiagaraComponent->Deactivate();
	return true;
}

bool UVFXExecutorComponent::StopVFXImmediate(const FVFXHandle& Handle)
{
	if (!Handle.IsValid())
		return false;
	
	FVFXInstanceData* Instance = ActiveInstances.Find(Handle);
	if (!Instance)
		return false;
	
	UNiagaraComponent* NiagaraComponent = Instance->NiagaraComponent.Get();
	ActiveInstances.Remove(Handle);
	
	if (IsValid(NiagaraComponent))
	{
		NiagaraComponent->OnSystemFinished.RemoveDynamic(this, &UVFXExecutorComponent::OnNiagaraFinished);
		NiagaraComponent->DeactivateImmediate();
		NiagaraComponent->DestroyComponent();
	}
	return true;
}

void UVFXExecutorComponent::OnNiagaraFinished(UNiagaraComponent* FinishedComponent)
{
	if (!IsValid(FinishedComponent))
		return;
	
	for (auto It = ActiveInstances.CreateIterator(); It; ++It)
	{
		if (It.Value().NiagaraComponent.Get() == FinishedComponent)
		{
			It.RemoveCurrent();
			break;
		}
	}
	
	FinishedComponent->OnSystemFinished.RemoveDynamic(this, &UVFXExecutorComponent::OnNiagaraFinished);
	FinishedComponent->DestroyComponent();
}

