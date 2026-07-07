#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GroupCombatSubsystem.generated.h"

UCLASS()
class PROJECT_MJS_API UGroupCombatSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	bool RequestAttackToken(AActor* Attacker, AActor* Target);
	
	void ReleaseAttackToken(AActor* Attacker);
	
	virtual void Deinitialize() override;

protected:
	// 플레이어(타겟)별로 현재 공격 중인 적들의 목록을 관리
	// Key: 타겟(플레이어), Value: 타겟을 공격 중인 적들의 배열
	TMap<AActor*, TArray<AActor*>> ActiveAttackersMap;

	// 난이도 조절용: 한 번에 플레이어를 동시에 공격할 수 있는 최대 근접 적의 수
	int32 MaxConcurrentAttackers = 5;
};