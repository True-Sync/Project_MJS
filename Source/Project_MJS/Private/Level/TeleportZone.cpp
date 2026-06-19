#include "Level/TeleportZone.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

ATeleportZone::ATeleportZone()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;

	TriggerBox->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ATeleportZone::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ATeleportZone::HandleTriggerBeginOverlap);
	}
}

void ATeleportZone::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || !OtherActor->IsA<APawn>())
	{
		return;
	}

	TeleportPawn(OtherActor);
}

bool ATeleportZone::TeleportPawn(AActor* TargetActor) const
{
	if (!TargetActor || !TeleportTargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportPawn failed: TargetActor or TeleportTargetActor is missing. Zone=%s"), *GetNameSafe(this));
		return false;
	}

	const FVector DestinationLocation = TeleportTargetActor->GetActorLocation();
	const FRotator DestinationRotation = bUseTargetRotation ? TeleportTargetActor->GetActorRotation() : TargetActor->GetActorRotation();

	return TargetActor->TeleportTo(DestinationLocation, DestinationRotation, false, true);
}
