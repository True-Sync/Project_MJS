#include "UI/PlayerStatusLayerWidget.h"
#include "Character/SharedComponent/HealthComponent.h"
#include "Components/ProgressBar.h"
#include "Math/UnrealMathUtility.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UPlayerStatusLayerWidget::InitHealthStatus(UHealthComponent* InHealthComp)
{
	if (CachedHealthComp.IsValid())
	{
		CachedHealthComp->OnHealthChanged.RemoveDynamic(this, &UPlayerStatusLayerWidget::HandleHealthChanged);
	}

	CachedHealthComp = InHealthComp;

	if (CachedHealthComp.IsValid())
	{
		CachedHealthComp->OnHealthChanged.AddDynamic(this, &UPlayerStatusLayerWidget::HandleHealthChanged);
		
		float InitialPercent = CachedHealthComp->GetHealthPercent();
		TargetHealthPercent = InitialPercent;
		CurrentCatchUpPercent = InitialPercent;

		if (ProgressBar_HP) ProgressBar_HP->SetPercent(InitialPercent);
		if (ProgressBar_HPCatchUp) ProgressBar_HPCatchUp->SetPercent(InitialPercent);
	}
}

void UPlayerStatusLayerWidget::HandleHealthChanged(float OldHealth, float NewHealth)
{
	if (!CachedHealthComp.IsValid()) return;

	TargetHealthPercent = CachedHealthComp->GetHealthPercent();

	if (ProgressBar_HP)
	{
		ProgressBar_HP->SetPercent(TargetHealthPercent);
	}

	if (NewHealth < OldHealth)
	{
		CatchUpTimer = 0.0f; 
		
		if (UWorld* World = GetWorld())
		{
			if (!World->GetTimerManager().IsTimerActive(CatchUpTimerHandle))
			{
				World->GetTimerManager().SetTimer(CatchUpTimerHandle, this, &UPlayerStatusLayerWidget::HandleCatchUpTick, 0.016f, true);
			}
		}
	}
	else
	{
		CurrentCatchUpPercent = TargetHealthPercent;
		if (ProgressBar_HPCatchUp) ProgressBar_HPCatchUp->SetPercent(CurrentCatchUpPercent);
		
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CatchUpTimerHandle);
		}
	}
}

void UPlayerStatusLayerWidget::HandleCatchUpTick()
{
	float DeltaTime = 0.016f; 
	CatchUpTimer += DeltaTime;

	if (CatchUpTimer >= CatchUpDelay)
	{
		CurrentCatchUpPercent = FMath::FInterpTo(CurrentCatchUpPercent, TargetHealthPercent, DeltaTime, CatchUpSpeed);

		if (ProgressBar_HPCatchUp)
		{
			ProgressBar_HPCatchUp->SetPercent(CurrentCatchUpPercent);
		}
		
		if (FMath::IsNearlyEqual(CurrentCatchUpPercent, TargetHealthPercent, 0.001f))
		{
			CurrentCatchUpPercent = TargetHealthPercent;
			if (ProgressBar_HPCatchUp) ProgressBar_HPCatchUp->SetPercent(CurrentCatchUpPercent);
			
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(CatchUpTimerHandle);
			}
		}
	}
}