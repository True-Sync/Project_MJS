#include "Character/Player/Component/TargetingComponent.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateHardTargetVisibility(DeltaTime);

	if (HasAutoTargetCandidate() || HardTarget.IsValid() || bRangedHardTargetAiming)
	{
		PublishTargetingDisplay();
	}

	if (bDrawDebugTargetingRange)
	{
		DrawDebugTargetingRange();
	}
}

void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeTargetRange();
	PublishTargetingDisplay();
	UpdateTargetingTimerState();
	UpdateComponentTickState();
}

void UTargetingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TargetingUpdateTimerHandle);
	}

	OnTargetingDisplayCleared.Broadcast();
	Super::EndPlay(EndPlayReason);
}

void UTargetingComponent::RequestHardTarget()
{
	RemoveInvalidCandidates();

	if (HardTarget.IsValid())
	{
		AActor* NextTarget = ChooseNextHardTarget();
		SetHardTarget(NextTarget, IsInsideAutoTargetRange(NextTarget) ? EHardTargetingMode::Auto : EHardTargetingMode::Ranged);
		return;
	}

	if (AutoTarget.IsValid())
	{
		SetHardTarget(AutoTarget.Get(), EHardTargetingMode::Auto);
	}
}

void UTargetingComponent::BeginRangedHardTargetAim()
{
	if (bRangedHardTargetAiming || HasAutoTargetCandidate())
	{
		return;
	}

	bRangedHardTargetAiming = true;
	UpdateComponentTickState();
	PublishTargetingDisplay();
}

void UTargetingComponent::CompleteRangedHardTargetAim()
{
	if (bRangedHardTargetAiming)
	{
		bRangedHardTargetAiming = false;

		if (AActor* RangedTarget = TraceRangedHardTarget())
		{
			SetHardTarget(RangedTarget, EHardTargetingMode::Ranged);
		}
	}

	PublishTargetingDisplay();
	UpdateComponentTickState();
}

void UTargetingComponent::CancelRangedHardTargetAim()
{
	bRangedHardTargetAiming = false;
	PublishTargetingDisplay();
	UpdateComponentTickState();
}

void UTargetingComponent::ClearHardTarget()
{
	SetHardTarget(nullptr, EHardTargetingMode::None);
	PublishTargetingDisplay();
}

AActor* UTargetingComponent::GetBestAttackTarget() const
{
	return HardTarget.IsValid() ? HardTarget.Get() : AutoTarget.Get();
}

void UTargetingComponent::OnTargetRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidCandidate(OtherActor))
	{
		return;
	}

	CandidateTargets.AddUnique(OtherActor);
	UpdateTargeting();
}

void UTargetingComponent::OnTargetRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CandidateTargets.Remove(OtherActor);
	UpdateTargeting();
}

void UTargetingComponent::InitializeTargetRange()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || TargetRangeSphere)
	{
		return;
	}

	TargetRangeSphere = NewObject<USphereComponent>(OwnerActor, TEXT("AutoTargetRangeSphere"));
	if (!TargetRangeSphere)
	{
		return;
	}

	TargetRangeSphere->SetupAttachment(OwnerActor->GetRootComponent());
	TargetRangeSphere->SetSphereRadius(HardTargetRadius);
	TargetRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TargetRangeSphere->SetCollisionObjectType(ECC_WorldDynamic);
	TargetRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	TargetRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TargetRangeSphere->SetCollisionResponseToChannel(RangedHardTargetTraceChannel, ECR_Ignore);
	TargetRangeSphere->SetGenerateOverlapEvents(true);
	TargetRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UTargetingComponent::OnTargetRangeBeginOverlap);
	TargetRangeSphere->OnComponentEndOverlap.AddDynamic(this, &UTargetingComponent::OnTargetRangeEndOverlap);
	TargetRangeSphere->RegisterComponent();
	TargetRangeSphere->UpdateOverlaps();
}

void UTargetingComponent::UpdateTargeting()
{
	RemoveInvalidCandidates();
	SetAutoTarget(ChooseAutoTarget());
	PublishTargetingDisplay();
	UpdateTargetingTimerState();
	UpdateComponentTickState();
}

void UTargetingComponent::UpdateTargetingTimerState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	const bool bShouldRunTimer = CandidateTargets.Num() > 0;

	if (bShouldRunTimer && !TimerManager.IsTimerActive(TargetingUpdateTimerHandle))
	{
		TimerManager.SetTimer(TargetingUpdateTimerHandle, this, &UTargetingComponent::UpdateTargeting, AutoTargetUpdateInterval, true);
	}
	else if (!bShouldRunTimer && TimerManager.IsTimerActive(TargetingUpdateTimerHandle))
	{
		TimerManager.ClearTimer(TargetingUpdateTimerHandle);
	}
}

void UTargetingComponent::PublishTargetingDisplay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	TArray<AActor*> DisplayTargets;
	for (const TWeakObjectPtr<AActor>& Candidate : CandidateTargets)
	{
		AActor* Target = Candidate.Get();
		if (ShouldDisplayMarkerForTarget(Target))
		{
			DisplayTargets.AddUnique(Target);
		}
	}

	if (ShouldDisplayMarkerForTarget(HardTarget.Get()))
	{
		DisplayTargets.AddUnique(HardTarget.Get());
	}

	TArray<FTargetingHUDMarkerData> MarkerDataList;
	for (AActor* Target : DisplayTargets)
	{
		if (!Target)
		{
			continue;
		}

		const FVector TargetWorldLocation = GetTargetWorldLocation(Target);

		FVector2D ScreenPosition;
		const bool bOnScreen = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PlayerController,
			TargetWorldLocation,
			ScreenPosition,
			true);
		if (!bOnScreen)
		{
			continue;
		}

		FTargetingHUDMarkerData MarkerData;
		MarkerData.TargetActor = Target;
		MarkerData.WorldLocation = TargetWorldLocation;
		MarkerData.ScreenPosition = ScreenPosition;
		MarkerData.MarkerType = ETargetingHUDMarkerType::Targetable;

		if (Target == HardTarget.Get())
		{
			MarkerData.MarkerType = ETargetingHUDMarkerType::HardTarget;
		}
		else if (Target == AutoTarget.Get())
		{
			MarkerData.MarkerType = ETargetingHUDMarkerType::AutoTarget;
		}

		MarkerDataList.Add(MarkerData);
	}

	OnTargetingDisplayUpdated.Broadcast(ShouldShowCrosshair(), MarkerDataList);
}

void UTargetingComponent::DrawDebugTargetingRange() const
{
#if ENABLE_DRAW_DEBUG
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	if (!OwnerActor || !World)
	{
		return;
	}

	const FVector Origin = OwnerActor->GetActorLocation();
	DrawDebugSphere(World, Origin, AutoTargetRadius, 48, AutoTargetDebugColor, false, 0.0f, 0, 1.5f);
	DrawDebugSphere(World, Origin, HardTargetRadius, 48, HardTargetDebugColor, false, 0.0f, 0, 1.5f);
#endif
}

void UTargetingComponent::UpdateComponentTickState()
{
	SetComponentTickEnabled(HasAutoTargetCandidate() || HardTarget.IsValid() || bRangedHardTargetAiming || bDrawDebugTargetingRange);
}

void UTargetingComponent::RemoveInvalidCandidates()
{
	for (int32 Index = CandidateTargets.Num() - 1; Index >= 0; --Index)
	{
		if (!IsValidCandidate(CandidateTargets[Index].Get()))
		{
			CandidateTargets.RemoveAtSwap(Index);
		}
	}

	if (HardTarget.IsValid() && !IsValidCandidate(HardTarget.Get()))
	{
		SetHardTarget(nullptr, EHardTargetingMode::None);
	}
}

void UTargetingComponent::SetAutoTarget(AActor* NewTarget)
{
	AutoTarget = NewTarget;
}

void UTargetingComponent::SetHardTarget(AActor* NewTarget, EHardTargetingMode NewMode)
{
	if (HardTarget.Get() == NewTarget && HardTargetingMode == (NewTarget ? NewMode : EHardTargetingMode::None))
	{
		return;
	}

	HardTarget = NewTarget;
	HardTargetingMode = NewTarget ? NewMode : EHardTargetingMode::None;
	HardTargetOutOfSightTime = 0.0f;

	OnHardTargetChanged.Broadcast(HardTarget.Get());
	UpdateComponentTickState();
}

void UTargetingComponent::UpdateHardTargetVisibility(float DeltaTime)
{
	if (!HardTarget.IsValid())
	{
		HardTargetOutOfSightTime = 0.0f;
		return;
	}

	if (IsTargetVisibleOnScreen(HardTarget.Get()))
	{
		HardTargetOutOfSightTime = 0.0f;
		return;
	}

	HardTargetOutOfSightTime += DeltaTime;
	if (HardTargetOutOfSightTime >= HardTargetOutOfSightGraceTime)
	{
		SetHardTarget(nullptr, EHardTargetingMode::None);
		PublishTargetingDisplay();
	}
}

bool UTargetingComponent::ShouldShowCrosshair() const
{
	return HasAutoTargetCandidate() || (bRangedHardTargetAiming && CandidateTargets.Num() > 0);
}

AActor* UTargetingComponent::ChooseAutoTarget() const
{
	AActor* DirectTarget = nullptr;
	float BestCrosshairDistance = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<AActor>& Candidate : CandidateTargets)
	{
		AActor* Target = Candidate.Get();
		if (!IsValidCandidate(Target) || !IsInsideAutoTargetRange(Target))
		{
			continue;
		}

		const float CrosshairDistance = GetCrosshairDistance(Target);
		if (CrosshairDistance <= DirectCrosshairPixelRadius && CrosshairDistance < BestCrosshairDistance)
		{
			BestCrosshairDistance = CrosshairDistance;
			DirectTarget = Target;
		}
	}

	if (DirectTarget)
	{
		return DirectTarget;
	}

	AActor* BestScoredTarget = nullptr;
	float BestScore = TNumericLimits<float>::Lowest();
	const AActor* OwnerActor = GetOwner();

	for (const TWeakObjectPtr<AActor>& Candidate : CandidateTargets)
	{
		AActor* Target = Candidate.Get();
		if (!OwnerActor || !IsValidCandidate(Target) || !IsInsideAutoTargetRange(Target))
		{
			continue;
		}

		const float CrosshairDistance = GetCrosshairDistance(Target);
		const float Distance = FVector::Dist(OwnerActor->GetActorLocation(), Target->GetActorLocation());
		const float CrosshairScore = 1.0f - FMath::Clamp(CrosshairDistance / ScreenCandidatePixelRadius, 0.0f, 1.0f);
		const float DistanceScore = 1.0f - FMath::Clamp(Distance / AutoTargetRadius, 0.0f, 1.0f);
		const float Score = CrosshairScore * 0.7f + DistanceScore * 0.3f;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestScoredTarget = Target;
		}
	}

	return BestScoredTarget;
}

AActor* UTargetingComponent::ChooseNextHardTarget() const
{
	TArray<AActor*> HardTargetCandidates;
	for (const TWeakObjectPtr<AActor>& Candidate : CandidateTargets)
	{
		AActor* Target = Candidate.Get();
		if (IsValidCandidate(Target) && IsInsideHardTargetRange(Target))
		{
			HardTargetCandidates.Add(Target);
		}
	}

	if (HardTargetCandidates.Num() == 0)
	{
		return nullptr;
	}

	HardTargetCandidates.Sort([this](const AActor& Left, const AActor& Right)
	{
		return GetCrosshairDistance(&Left) < GetCrosshairDistance(&Right);
	});

	const int32 CurrentIndex = HardTarget.IsValid() ? HardTargetCandidates.IndexOfByKey(HardTarget.Get()) : INDEX_NONE;
	return HardTargetCandidates.IsValidIndex(CurrentIndex + 1) ? HardTargetCandidates[CurrentIndex + 1] : HardTargetCandidates[0];
}

AActor* UTargetingComponent::TraceRangedHardTarget() const
{
	const AActor* OwnerActor = GetOwner();
	const APlayerController* PlayerController = GetOwningPlayerController();
	if (!OwnerActor || !PlayerController)
	{
		return nullptr;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

	FVector TraceStart;
	FVector TraceDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f, TraceStart, TraceDirection))
	{
		if (bLogRangedHardTargetTrace)
		{
			UE_LOG(LogTemp, Warning, TEXT("RangedHardTargetTrace failed: screen center deprojection failed."));
		}
		return nullptr;
	}

	const float CameraToOwnerDistance = FVector::Dist(TraceStart, OwnerActor->GetActorLocation());

	const float TraceLength = HardTargetRadius + CameraToOwnerDistance + RangedHardTargetTraceExtraLength;
	const FVector TraceEnd = TraceStart + TraceDirection * TraceLength;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RangedHardTargetTrace), bUseComplexCollisionForRangedTrace);
	QueryParams.AddIgnoredActor(OwnerActor);
	if (TargetRangeSphere)
	{
		QueryParams.AddIgnoredComponent(TargetRangeSphere.Get());
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		if (bLogRangedHardTargetTrace)
		{
			UE_LOG(LogTemp, Warning, TEXT("RangedHardTargetTrace failed: World is missing."));
		}
		return nullptr;
	}

	const bool bHit = bUseRangedHardTargetSphereTrace
		? World->SweepSingleByChannel(HitResult, TraceStart, TraceEnd, FQuat::Identity, RangedHardTargetTraceChannel, FCollisionShape::MakeSphere(RangedHardTargetTraceRadius), QueryParams)
		: World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, RangedHardTargetTraceChannel, QueryParams);
	AActor* HitActor = HitResult.GetActor();
	const bool bValidCandidate = IsValidCandidate(HitActor);
	const bool bInsideHardTargetRange = IsInsideHardTargetRange(HitActor);
	const bool bAccepted = bHit && bValidCandidate && bInsideHardTargetRange;
	const bool bHitTargetRangeSphere = HitResult.GetComponent() && HitResult.GetComponent() == TargetRangeSphere;

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugRangedHardTargetTrace)
	{
		const FVector DebugEnd = bHit ? HitResult.ImpactPoint : TraceEnd;
		const FColor DebugColor = bAccepted ? RangedHardTargetTraceHitColor : (bHit ? RangedHardTargetTraceRejectedColor : RangedHardTargetTraceMissColor);
		DrawDebugLine(World, TraceStart, DebugEnd, DebugColor, false, RangedHardTargetTraceDebugDuration, 0, 2.0f);
		DrawDebugSphere(World, DebugEnd, bUseRangedHardTargetSphereTrace ? RangedHardTargetTraceRadius : 14.0f, 12, DebugColor, false, RangedHardTargetTraceDebugDuration);
	}
#endif

	if (bLogRangedHardTargetTrace)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("RangedHardTargetTrace: Hit=%s Accepted=%s Actor=%s Component=%s Distance=%.1f ValidCandidate=%s InsideHardRange=%s HitTargetRangeSphere=%s Channel=%d Shape=%s Radius=%.1f TraceLength=%.1f HardRange=%.1f CameraToOwner=%.1f Start=%s End=%s"),
			bHit ? TEXT("true") : TEXT("false"),
			bAccepted ? TEXT("true") : TEXT("false"),
			*GetNameSafe(HitActor),
			*GetNameSafe(HitResult.GetComponent()),
			bHit ? HitResult.Distance : TraceLength,
			bValidCandidate ? TEXT("true") : TEXT("false"),
			bInsideHardTargetRange ? TEXT("true") : TEXT("false"),
			bHitTargetRangeSphere ? TEXT("true") : TEXT("false"),
			static_cast<int32>(RangedHardTargetTraceChannel.GetValue()),
			bUseRangedHardTargetSphereTrace ? TEXT("Sphere") : TEXT("Line"),
			bUseRangedHardTargetSphereTrace ? RangedHardTargetTraceRadius : 0.0f,
			TraceLength,
			HardTargetRadius,
			CameraToOwnerDistance,
			*TraceStart.ToCompactString(),
			*TraceEnd.ToCompactString());
	}

	return bAccepted ? HitActor : nullptr;
}

APlayerController* UTargetingComponent::GetOwningPlayerController() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
}

bool UTargetingComponent::IsValidCandidate(AActor* Candidate) const
{
	return Candidate && Candidate != GetOwner() && Candidate->IsA<AEnemyCharacter>();
}

bool UTargetingComponent::IsInsideAutoTargetRange(AActor* Candidate) const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && Candidate && FVector::DistSquared(OwnerActor->GetActorLocation(), Candidate->GetActorLocation()) <= FMath::Square(AutoTargetRadius);
}

bool UTargetingComponent::IsInsideHardTargetRange(AActor* Candidate) const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && Candidate && FVector::DistSquared(OwnerActor->GetActorLocation(), Candidate->GetActorLocation()) <= FMath::Square(HardTargetRadius);
}

bool UTargetingComponent::HasAutoTargetCandidate() const
{
	for (const TWeakObjectPtr<AActor>& Candidate : CandidateTargets)
	{
		if (IsValidCandidate(Candidate.Get()) && IsInsideAutoTargetRange(Candidate.Get()))
		{
			return true;
		}
	}

	return false;
}

bool UTargetingComponent::ShouldDisplayMarkerForTarget(AActor* Target) const
{
	return IsValidCandidate(Target) && (Target == HardTarget.Get() || IsInsideAutoTargetRange(Target));
}

bool UTargetingComponent::IsTargetVisibleOnScreen(AActor* Target) const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !Target)
	{
		return false;
	}

	FVector2D ScreenPosition;
	return PlayerController->ProjectWorldLocationToScreen(GetTargetWorldLocation(Target), ScreenPosition, true);
}

float UTargetingComponent::GetCrosshairDistance(const AActor* Candidate) const
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController || !Candidate)
	{
		return TNumericLimits<float>::Max();
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
	const FVector2D CrosshairPosition(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);

	FVector2D ScreenPosition;
	if (!PlayerController->ProjectWorldLocationToScreen(GetTargetWorldLocation(Candidate), ScreenPosition, true))
	{
		return TNumericLimits<float>::Max();
	}

	return FVector2D::Distance(CrosshairPosition, ScreenPosition);
}

FVector UTargetingComponent::GetTargetWorldLocation(const AActor* Target) const
{
	if (const AEnemyCharacter* EnemyCharacter = Cast<AEnemyCharacter>(Target))
	{
		return EnemyCharacter->GetTargetPointLocation();
	}

	return Target ? Target->GetActorLocation() : FVector::ZeroVector;
}
