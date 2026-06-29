#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyFSMComponent.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Approach,
	Attack_Telegraph,
	Attack_Execution,
	Attack_Recovery,
	Stagger,
	Return
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECT_MJS_API UEnemyFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEnemyFSMComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "AI|FSM")
	void ChangeState(EEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI|FSM")
	EEnemyState GetCurrentState() const { return CurrentState; }

	// 오픈월드 최적화를 위한 초기 스폰 위치
	UPROPERTY(BlueprintReadOnly, Category = "AI|FSM")
	FVector SpawnLocation;

	UPROPERTY(BlueprintReadWrite, Category = "AI|FSM")
	TObjectPtr<AActor> TargetPlayer;

private:
	EEnemyState CurrentState = EEnemyState::Idle;
	class AEnemyCharacter* OwnerCharacter;
	class AAIController* OwnerAIController;

	void UpdateIdle();
	void UpdateApproach();
	void UpdateReturn();
	bool CheckLeashDistance();
};