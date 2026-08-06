#include "CommandBox/CommandBoxActor.h"

#include "Character/Player/CPlayerController.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "Interaction/InteractionComponent.h"

ACommandBoxActor::ACommandBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	CommandBoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CommandBoxMesh"));
	CommandBoxMesh->SetupAttachment(SceneRoot);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(SceneRoot);
	InteractionVolume->SetBoxExtent(FVector(180.0f, 180.0f, 120.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionVolume->SetGenerateOverlapEvents(true);

	InteractionPrompt = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionPrompt"));
	InteractionPrompt->SetupAttachment(SceneRoot);
	InteractionPrompt->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));
	InteractionPrompt->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionPrompt->SetDrawAtDesiredSize(true);
	InteractionPrompt->SetTickMode(ETickMode::Enabled);
	InteractionPrompt->SetVisibility(false);
}

void ACommandBoxActor::BeginPlay()
{
	Super::BeginPlay();

	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ACommandBoxActor::HandleInteractionBeginOverlap);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ACommandBoxActor::HandleInteractionEndOverlap);
	SetInteractionPromptVisible_Implementation(false);
}

bool ACommandBoxActor::CanInteract_Implementation(APawn* InteractingPawn) const
{
	return bInteractionEnabled && IsValid(ResolvePlayerController(InteractingPawn));
}

void ACommandBoxActor::Interact_Implementation(APawn* InteractingPawn)
{
	ACPlayerController* PlayerController = ResolvePlayerController(InteractingPawn);
	if (!bInteractionEnabled || !PlayerController)
	{
		return;
	}

	PlayerController->OpenCommandBoxMenu(this);
}

void ACommandBoxActor::SetInteractionPromptVisible_Implementation(bool bVisible)
{
	if (InteractionPrompt)
	{
		InteractionPrompt->SetVisibility(bVisible && bInteractionEnabled);
	}
}

void ACommandBoxActor::RequestHousing(ACPlayerController* PlayerController)
{
	OnHousingRequested.Broadcast(this, PlayerController);
}

void ACommandBoxActor::RequestCostume(ACPlayerController* PlayerController)
{
	OnCostumeRequested.Broadcast(this, PlayerController);
}

void ACommandBoxActor::RequestStageTravel(ACPlayerController* PlayerController)
{
	OnStageTravelRequested.Broadcast(this, PlayerController);
}

void ACommandBoxActor::HandleInteractionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	UInteractionComponent* InteractionComponent = ResolveInteractionComponent(OtherActor);
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->RegisterInteractable(this);
}

void ACommandBoxActor::HandleInteractionEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (InteractionVolume->IsOverlappingActor(OtherActor))
	{
		return;
	}

	if (UInteractionComponent* InteractionComponent = ResolveInteractionComponent(OtherActor))
	{
		InteractionComponent->UnregisterInteractable(this);
	}
}

ACPlayerController* ACommandBoxActor::ResolvePlayerController(const APawn* Pawn) const
{
	return Pawn ? Cast<ACPlayerController>(Pawn->GetController()) : nullptr;
}

UInteractionComponent* ACommandBoxActor::ResolveInteractionComponent(AActor* OtherActor) const
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	return Pawn ? Pawn->FindComponentByClass<UInteractionComponent>() : nullptr;
}
