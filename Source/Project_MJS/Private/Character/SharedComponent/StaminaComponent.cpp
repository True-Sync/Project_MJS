// StaminaComponent: improved delegate usage, depletion guard, and ConfigureStaminaBehavior signature (v2).
#include "Character/SharedComponent/StaminaComponent.h"
#include "Components/StaminaCostData.h"

#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	if (CurrentStamina <= 0.0f || CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}

	OnStaminaChanged.Broadcast(0.0f, CurrentStamina);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentStamina >= MaxStamina)
	{
		return;
	}

	RegenerateStamina(DeltaTime);
}

float UStaminaComponent::GetStaminaPercent() const
{
	if (MaxStamina <= 0.0f)
	{
		return 0.0f;
	}
	return FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f);
}

bool UStaminaComponent::ConsumeStamina(const FStaminaCostData& CostData)
{
	if (!CostData.bCanConsume || CurrentStamina <= 0.0f)
	{
		return false;
	}

	float Amount = CostData.StaminaCost;
	
	// 스태미나가 부족하면 소모하지 않고 실패 반환
	if (CurrentStamina < Amount)
	{
		return false;
	}

	// 1. 스태미나 소모
	ApplyStaminaDelta(-Amount);

	// 2. 소모 중 회복량 처리 (점프, 달리기 등 지속적 소모 시)
	if (CostData.StaminaRegenRate > 0.0f)
	{
		const float OldStamina = CurrentStamina;
		const float RegenAmount = CostData.StaminaRegenRate * 0.1f; // 0.1 초 기준
		CurrentStamina = FMath::Min(CurrentStamina + RegenAmount, MaxStamina);

		if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
		{
			OnStaminaChanged.Broadcast(OldStamina, CurrentStamina);
		}
	}

	// 3. 최대 스태미나 증가 효과 적용 (일시적 버프)
	if (CostData.StaminaMaxIncrease > 0.0f && CostData.StaminaDuration > 0.0f)
	{
		const float OldMaxStamina = MaxStamina;
		MaxStamina += CostData.StaminaMaxIncrease;

		// 최대 스태미나 증가 시 현재 스태미나도 함께 증가 (예: 스킬 사용 시 현재 스태미나 +50%)
		if (CurrentStamina < MaxStamina)
		{
			const float BonusStamina = FMath::Min(CurrentStamina * 0.5f, CostData.StaminaMaxIncrease);
			ApplyStaminaDelta(BonusStamina);
		}

		// 상태 효과 적용 (Blueprint 에서 구현)
		if (!CostData.EffectTags.IsEmpty())
		{
			OnEffectApplied.Broadcast(CostData.EffectTags, CostData.StaminaDuration);
		}

		// 상태 효과 제거
		if (!CostData.RemovedEffectTags.IsEmpty())
		{
			OnEffectRemoved.Broadcast(CostData.RemovedEffectTags);
		}

		// 지속 시간 종료 시 최대 스태미나 복원
		FTimerHandle MaxStaminaRestoreHandle;
		GetWorld()->GetTimerManager().SetTimer(
			MaxStaminaRestoreHandle,
			FTimerDelegate::CreateLambda([this, OldMaxStamina]()
			{
				const float NewMaxStamina = OldMaxStamina;
				MaxStamina = NewMaxStamina;
				OnMaxStaminaRestored.Broadcast(OldMaxStamina, NewMaxStamina);
			}),
			CostData.StaminaDuration,
			false
		);
	}

	return true;
}

bool UStaminaComponent::ConsumeStamina(float Amount)
{
	// 레거시 호환성 - 기본값 사용
	FStaminaCostData DefaultCost;
	DefaultCost.StaminaCost = Amount;
	DefaultCost.bCanConsume = true;
	return ConsumeStamina(DefaultCost);
}

void UStaminaComponent::RegenerateStamina(float DeltaTime)
{
	if (CurrentStamina >= MaxStamina || StaminaRegenRate <= 0.0f || DeltaTime <= 0.0f)
	{
		return;
	}

	const float OldStamina = CurrentStamina;
	const float RegenAmount = StaminaRegenRate * DeltaTime;
	CurrentStamina = FMath::Min(CurrentStamina + RegenAmount, MaxStamina);

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(OldStamina, CurrentStamina);
	}
}

void UStaminaComponent::ApplyStaminaDelta(float Delta)
{
	const float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Delta, 0.0f, MaxStamina);

	if (!FMath::IsNearlyEqual(OldStamina, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(OldStamina, CurrentStamina);
	}

	// 스태미나가 고갈되고 아직 처리되지 않았다면 한 번만 처리
	if (CurrentStamina <= 0.0f && !bIsDepleted)
	{
		HandleStaminaDepletion();
	}
}

void UStaminaComponent::HandleStaminaDepletion()
{
	if (bIsDepleted)
	{
		return;
	}

	bIsDepleted = true;
	OnStaminaDepleted.Broadcast();

	if (!bDepleteOnDeath)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->Destroy();
	}
}

void UStaminaComponent::ConfigureStaminaBehavior(float InRegenRate, bool bInDepleteOnDeath)
{
	StaminaRegenRate = InRegenRate;
	this->bDepleteOnDeath = bInDepleteOnDeath;
}
