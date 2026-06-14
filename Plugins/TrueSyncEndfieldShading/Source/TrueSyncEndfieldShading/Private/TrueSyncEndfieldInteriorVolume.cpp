#include "TrueSyncEndfieldInteriorVolume.h"

#include "TrueSyncEndfieldShadingWorldSubsystem.h"
#include "Components/BoxComponent.h"

ATrueSyncEndfieldInteriorVolume::ATrueSyncEndfieldInteriorVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->InitBoxExtent(FVector(600.0f, 600.0f, 300.0f));
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoxComponent->SetGenerateOverlapEvents(false);
	BoxComponent->SetCanEverAffectNavigation(false);
	BoxComponent->SetHiddenInGame(true);
	RootComponent = BoxComponent;

	BlendDistance = 300.0f;
}

void ATrueSyncEndfieldInteriorVolume::BeginPlay()
{
	Super::BeginPlay();
	RegisterWithSubsystem();
}

void ATrueSyncEndfieldInteriorVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterWithSubsystem();
	Super::EndPlay(EndPlayReason);
}

void ATrueSyncEndfieldInteriorVolume::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	RegisterWithSubsystem();
}

void ATrueSyncEndfieldInteriorVolume::PostUnregisterAllComponents()
{
	UnregisterWithSubsystem();
	Super::PostUnregisterAllComponents();
}

float ATrueSyncEndfieldInteriorVolume::ComputeBlendWeight(const FVector& WorldLocation) const
{
	if (!BoxComponent)
	{
		return 0.0f;
	}

	const FVector LocalPosition = BoxComponent->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector AbsLocalPosition = LocalPosition.GetAbs();
	const FVector Extent = BoxComponent->GetScaledBoxExtent();

	const FVector OutsideDistance(
		FMath::Max(AbsLocalPosition.X - Extent.X, 0.0f),
		FMath::Max(AbsLocalPosition.Y - Extent.Y, 0.0f),
		FMath::Max(AbsLocalPosition.Z - Extent.Z, 0.0f));

	const float DistanceToBox = OutsideDistance.Size();
	if (DistanceToBox <= UE_KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	if (BlendDistance <= UE_KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return 1.0f - FMath::Clamp(DistanceToBox / BlendDistance, 0.0f, 1.0f);
}

void ATrueSyncEndfieldInteriorVolume::RegisterWithSubsystem()
{
	if (bRegisteredWithSubsystem)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UTrueSyncEndfieldShadingWorldSubsystem* Subsystem =
			World->GetSubsystem<UTrueSyncEndfieldShadingWorldSubsystem>())
		{
			Subsystem->RegisterInteriorVolume(this);
			bRegisteredWithSubsystem = true;
		}
	}
}

void ATrueSyncEndfieldInteriorVolume::UnregisterWithSubsystem()
{
	if (!bRegisteredWithSubsystem)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UTrueSyncEndfieldShadingWorldSubsystem* Subsystem =
			World->GetSubsystem<UTrueSyncEndfieldShadingWorldSubsystem>())
		{
			Subsystem->UnregisterInteriorVolume(this);
		}
	}

	bRegisteredWithSubsystem = false;
}
