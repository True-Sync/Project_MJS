// StaminaStatusEffect: 상태 효과 시스템 기본 구현
#include "Components/StaminaStatusEffect.h"

UStaminaStatusEffect::UStaminaStatusEffect()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaStatusEffect::BeginPlay()
{
	Super::BeginPlay();

	// 시작 시 적용된 상태 효과 처리
	for (auto& Effect : ActiveStatusEffects)
	{
		if (Effect.bApplyOnStart)
		{
			ApplyStatusEffect(Effect);
		}
	}
}

void UStaminaStatusEffect::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateStatusEffects(DeltaTime);
}

void UStaminaStatusEffect::ApplyStatusEffect(const FStaminaStatusEffectData& Data)
{
	// 이미 적용된 상태 효과인지 확인
	for (auto& Effect : ActiveStatusEffects)
	{
		if (Effect.Tags.HasAny(Data.Tags))
		{
			// 기존 효과를 업데이트
			Effect.Duration = Data.Duration;
			Effect.StaminaModifier = Data.StaminaModifier;
			return;
		}
	}

	ActiveStatusEffects.Add(Data);
}

void UStaminaStatusEffect::RemoveStatusEffect(const FGameplayTagContainer& Tags)
{
	ActiveStatusEffects.RemoveAll([&Tags](const FStaminaStatusEffectData& Effect)
	{
		return Effect.Tags.HasAny(Tags);
	});
}

TArray<FStaminaStatusEffectData> UStaminaStatusEffect::GetActiveStatusEffects() const
{
	return ActiveStatusEffects;
}

void UStaminaStatusEffect::UpdateStatusEffects(float DeltaTime)
{
	for (int32 i = ActiveStatusEffects.Num() - 1; i >= 0; --i)
	{
		FStaminaStatusEffectData& Effect = ActiveStatusEffects[i];

		// 지속 시간 감소
		Effect.Duration -= DeltaTime;

		if (Effect.Duration <= 0.0f)
		{
			// 지속 시간 종료
			if (Effect.bRemoveOnEnd)
			{
				ActiveStatusEffects.RemoveAt(i);
			}
		}
	}
}