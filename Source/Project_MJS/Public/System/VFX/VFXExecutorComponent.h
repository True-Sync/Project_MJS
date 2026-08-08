#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "System/VFX/VFXTypes.h"
#include "VFXExecutorComponent.generated.h"

class UCharacterVFXProfile;
class UNiagaraComponent;

USTRUCT()
struct FVFXInstanceData
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> NiagaraComponent = nullptr;

	UPROPERTY(Transient)
	FGameplayTag Tag;
};

UCLASS(ClassGroup = (VFX), meta = (BlueprintSpawnableComponent))
class PROJECT_MJS_API UVFXExecutorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVFXExecutorComponent();
	
	UFUNCTION(BlueprintCallable, Category = "VFX")
	FVFXHandle  ExecuteVFX(const FGameplayTag& Tag, const FVFXExecuteContext& Context);
	
	//기존 파티클 자연스럽게 종료
	UFUNCTION(BlueprintCallable, Category = "VFX")
	bool StopVFX(const FVFXHandle& Handle);
	
	//즉시 제거
	UFUNCTION(BlueprintCallable, Category = "VFX") 
	bool StopVFXImmediate(const FVFXHandle& Handle);
	
private:
	// 이펙트 재생 종료판정
	UFUNCTION()
	void OnNiagaraFinished(UNiagaraComponent* FinishedComponent);
	
	UNiagaraComponent* SpawnNiagara(const FGameplayTag& Tag, const FVFXDefinition& ExecuteDefinition, const FVFXExecuteContext& Context);
	void ApplyFloatParameters(UNiagaraComponent* NiagaraComponent, const FVFXDefinition& Definition, const FVFXExecuteContext& Context);
	
	//======== 변수 =========
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UCharacterVFXProfile> CharacterVfxProfile;
	
private:
	UPROPERTY(Transient)
	TMap<FVFXHandle, FVFXInstanceData> ActiveInstances;
};