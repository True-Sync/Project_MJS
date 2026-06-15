#include "Cinematic/CinematicTriggerActor.h"

#include "Cinematic/CinematicDirectorSubsystem.h"
#include "Cinematic/CinematicTypes.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "LevelSequence.h"

ACinematicTriggerActor::ACinematicTriggerActor()
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

void ACinematicTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACinematicTriggerActor::HandleTriggerBeginOverlap);
	}
}

bool ACinematicTriggerActor::ActivateCinematic(AActor* TargetActor)
{
	if (!CanTriggerFor(TargetActor))
	{
		return false;
	}

	UWorld* World = GetWorld();
	UCinematicDirectorSubsystem* DirectorSubsystem = World ? World->GetSubsystem<UCinematicDirectorSubsystem>() : nullptr;
	if (!DirectorSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateCinematic failed: CinematicDirectorSubsystem is missing."));
		return false;
	}

	FCinematicPlaybackRequest Request;
	Request.Sequence = Sequence;
	Request.InstigatorActor = TargetActor;
	Request.SubjectActor = TargetActor;
	Request.bAffectAllParticipants = bAffectAllParticipants;
	Request.bRestoreViewTarget = bRestoreViewTarget;
	Request.BlendOutTime = BlendOutTime;
	Request.NetworkPolicy = NetworkPolicy;

	if (bBindTriggeringActor && !TriggeringActorBindingTag.IsNone())
	{
		FCinematicBindingOverride TriggeringActorBindingOverride;
		TriggeringActorBindingOverride.BindingTag = TriggeringActorBindingTag;
		TriggeringActorBindingOverride.Actors.Add(TargetActor);
		Request.BindingOverrides.Add(TriggeringActorBindingOverride);
	}

	Request.BindingOverrides.Append(BindingOverrides);
	Request.AnchorMode = AnchorMode;
	Request.RotationSource = RotationSource;
	Request.AnchorActor = bUseTriggerActorAsAnchor ? this : nullptr;
	Request.AnchorSocketName = AnchorSocketName;
	Request.TargetSocketName = TargetSocketName;
	Request.RelativeTransform = RelativeTransform;
	Request.ExplicitWorldTransform = ExplicitWorldTransform;
	Request.ExplicitRotation = ExplicitRotation;
	Request.bUseYawOnly = bUseYawOnly;
	Request.bDrawDebugAnchor = bDrawDebugAnchor;
	Request.DebugDrawDuration = DebugDrawDuration;
	Request.DebugDrawScale = DebugDrawScale;

	StopTargetActiveMontages(TargetActor);

	if (!DirectorSubsystem->PlayCinematic(Request))
	{
		return false;
	}

	bHasTriggered = true;
	return true;
}

void ACinematicTriggerActor::SetTriggerEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
}

void ACinematicTriggerActor::ResetTrigger()
{
	bHasTriggered = false;
	bEnabled = true;
}

void ACinematicTriggerActor::HandleTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bTriggerOnOverlap)
	{
		return;
	}

	ActivateCinematic(OtherActor);
}

bool ACinematicTriggerActor::CanTriggerFor(AActor* TargetActor) const
{
	if (!bEnabled || !Sequence || !TargetActor)
	{
		return false;
	}

	if (bTriggerOnce && bHasTriggered)
	{
		return false;
	}

	if (RequiredActorClass && !TargetActor->IsA(RequiredActorClass))
	{
		return false;
	}

	return true;
}

void ACinematicTriggerActor::StopTargetActiveMontages(AActor* TargetActor) const
{
	if (!bStopTargetMontagesOnTrigger || !TargetActor)
	{
		return;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	TargetActor->GetComponents(SkeletalMeshComponents);

	for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
	{
		UAnimInstance* AnimInstance = SkeletalMeshComponent ? SkeletalMeshComponent->GetAnimInstance() : nullptr;
		if (AnimInstance)
		{
			AnimInstance->Montage_Stop(0.05f);
		}
	}
}
