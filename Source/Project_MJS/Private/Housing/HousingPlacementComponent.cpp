#include "Housing/HousingPlacementComponent.h"

#include "Character/Player/CPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Housing/HousingAreaActor.h"
#include "Housing/HousingCameraActor.h"
#include "Housing/HousingItemDataAsset.h"
#include "Interaction/InteractionComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

UHousingPlacementComponent::UHousingPlacementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UHousingPlacementComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
}

void UHousingPlacementComponent::InitializeInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (IA_ToggleCatalog)
	{
		EnhancedInputComponent->BindAction(IA_ToggleCatalog, ETriggerEvent::Started, this, &UHousingPlacementComponent::ToggleCatalog);
	}

	if (IA_ExitHousing)
	{
		EnhancedInputComponent->BindAction(IA_ExitHousing, ETriggerEvent::Started, this, &UHousingPlacementComponent::HandleExitInput);
	}

	if (IA_RotatePlacement)
	{
		EnhancedInputComponent->BindAction(IA_RotatePlacement, ETriggerEvent::Started, this, &UHousingPlacementComponent::RotatePlacementClockwise);
	}

	if (IA_ConfirmPlacement)
	{
		EnhancedInputComponent->BindAction(IA_ConfirmPlacement, ETriggerEvent::Started, this, &UHousingPlacementComponent::HandleConfirmInput);
	}

	if (IA_SelectPlacedProp)
	{
		EnhancedInputComponent->BindAction(IA_SelectPlacedProp, ETriggerEvent::Started, this, &UHousingPlacementComponent::HandleSelectPlacedPropInput);
	}

	if (IA_CancelSelectPlacedProp)
	{
		EnhancedInputComponent->BindAction(IA_CancelSelectPlacedProp, ETriggerEvent::Started, this, &UHousingPlacementComponent::HandleCancelSelectPlacedPropInput);
	}

	if (IA_RecallPlacement)
	{
		EnhancedInputComponent->BindAction(IA_RecallPlacement, ETriggerEvent::Started, this, &UHousingPlacementComponent::RecallPlacement);
	}
}

bool UHousingPlacementComponent::EnterHousing(AHousingAreaActor* HousingArea)
{
	ACPlayerController* PlayerController = GetPlayerController();
	if (IsHousingActive() || !IsValid(HousingArea) || !PlayerController)
	{
		return false;
	}

	AHousingCameraActor* HousingCamera = HousingArea->GetHousingCamera();
	if (!IsValid(HousingCamera))
	{
		UE_LOG(LogTemp, Warning, TEXT("EnterHousing failed: HousingCamera is not assigned on %s."), *GetNameSafe(HousingArea));
		return false;
	}

	ActiveArea = HousingArea;
	PreviousViewTarget = PlayerController->GetViewTarget();
	AddHousingInputMapping();

	HousingArea->SetGridVisible(true);
	PlayerController->SetViewTargetWithBlend(HousingCamera, HousingCamera->GetBlendTime());
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	PlayerController->bShowMouseCursor = true;
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		if (UInteractionComponent* InteractionComponent = Pawn->FindComponentByClass<UInteractionComponent>())
		{
			InteractionComponent->SetInteractionPromptEnabled(false);
		}
	}

	SetState(EHousingPlacementState::Browsing);
	return true;
}

void UHousingPlacementComponent::ExitHousing()
{
	if (!IsHousingActive())
	{
		return;
	}

	if (State == EHousingPlacementState::CatalogOpen)
	{
		OnCatalogVisibilityChanged.Broadcast(false);
	}

	if (State == EHousingPlacementState::Previewing)
	{
		CancelPlacement();
	}
	else
	{
		DestroyPreview();
	}

	ACPlayerController* PlayerController = GetPlayerController();
	if (AHousingAreaActor* HousingArea = ActiveArea.Get())
	{
		HousingArea->SetGridVisible(false);
	}

	RemoveHousingInputMapping();

	if (PlayerController)
	{
		if (AActor* ViewTarget = PreviousViewTarget.Get())
		{
			PlayerController->SetViewTargetWithBlend(ViewTarget, 0.25f);
		}

		PlayerController->SetIgnoreMoveInput(false);
		PlayerController->SetIgnoreLookInput(false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->bEnableClickEvents = false;
		PlayerController->bEnableMouseOverEvents = false;
		PlayerController->SetInputMode(FInputModeGameOnly());

		if (APawn* Pawn = PlayerController->GetPawn())
		{
			if (UInteractionComponent* InteractionComponent = Pawn->FindComponentByClass<UInteractionComponent>())
			{
				InteractionComponent->SetInteractionPromptEnabled(true);
			}
		}
	}

	ActiveArea.Reset();
	PreviousViewTarget.Reset();
	SetState(EHousingPlacementState::Inactive);
}

void UHousingPlacementComponent::ToggleCatalog()
{
	if (State == EHousingPlacementState::Previewing)
	{
		CancelPlacement();
		SetState(EHousingPlacementState::CatalogOpen);
		OnCatalogVisibilityChanged.Broadcast(true);
		return;
	}

	if (State == EHousingPlacementState::Browsing)
	{
		SetState(EHousingPlacementState::CatalogOpen);
		OnCatalogVisibilityChanged.Broadcast(true);
	}
	else if (State == EHousingPlacementState::CatalogOpen)
	{
		SetState(EHousingPlacementState::Browsing);
		OnCatalogVisibilityChanged.Broadcast(false);
	}
}

bool UHousingPlacementComponent::BeginPlacement(UHousingItemDataAsset* ItemData)
{
	if (State != EHousingPlacementState::CatalogOpen || !IsItemAvailable(ItemData))
	{
		return false;
	}

	DestroyPreview();
	PreviewActor = SpawnPreviewActor(ItemData);
	if (!PreviewActor)
	{
		return false;
	}

	PreparePreviewActor();

	UE_LOG(LogTemp, Log, TEXT("Housing actor preview selected: Data=%s, PreviewActor=%s, PlacedClass=%s"),
		*GetNameSafe(ItemData),
		*GetNameSafe(PreviewActor),
		*ItemData->PlacedActorClass.ToSoftObjectPath().ToString());

	SelectedItem = ItemData;
	CurrentRotationQuarterTurns = 0;
	bCurrentPlacementValid = false;
	bLastAppliedPreviewValidity = true;
	ApplyPreviewMaterial(false);
	OnCatalogVisibilityChanged.Broadcast(false);
	SetState(EHousingPlacementState::Previewing);
	SetComponentTickEnabled(true);
	UpdatePreviewFromCursor();
	return true;
}

bool UHousingPlacementComponent::IsItemAvailable(const UHousingItemDataAsset* ItemData) const
{
	const AHousingAreaActor* HousingArea = ActiveArea.Get();
	return IsValid(ItemData) && HousingArea && !HousingArea->IsItemPlaced(ItemData);
}

EHousingGuideContext UHousingPlacementComponent::GetGuideContext() const
{
	switch (State)
	{
	case EHousingPlacementState::Browsing:
		return EHousingGuideContext::Browsing;
	case EHousingPlacementState::CatalogOpen:
		return EHousingGuideContext::CatalogOpen;
	case EHousingPlacementState::Previewing:
		return bMovingExistingPlacement
			? EHousingGuideContext::MovingPlacedProp
			: EHousingGuideContext::NewPlacement;
	default:
		return EHousingGuideContext::Hidden;
	}
}

void UHousingPlacementComponent::CancelPlacement()
{
	if (State != EHousingPlacementState::Previewing)
	{
		return;
	}

	if (bMovingExistingPlacement && PreviewActor)
	{
		AHousingAreaActor* HousingArea = ActiveArea.Get();
		AActor* MovingActor = PreviewActor;
		MovingActor->SetActorTransform(OriginalActorTransform);
		if (!HousingArea || !HousingArea->RegisterPlacement(
			OriginalPlacementRecord,
			OriginalOccupiedCells,
			MovingActor,
			SelectedItem))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to restore canceled housing placement for %s."), *GetNameSafe(MovingActor));
			return;
		}

		RestorePreviewActorState();
		MovingActor->Tags.Remove(TEXT("HousingPlacementPreview"));
		PreviewActor = nullptr;
		SetComponentTickEnabled(false);
		ResetPlacementData();
	}
	else
	{
		DestroyPreview();
	}
	SetState(EHousingPlacementState::Browsing);
}

void UHousingPlacementComponent::RotatePlacementClockwise()
{
	if (State != EHousingPlacementState::Previewing || !SelectedItem || !SelectedItem->bCanRotate)
	{
		return;
	}

	CurrentRotationQuarterTurns = (CurrentRotationQuarterTurns + 1) % 4;
	UpdatePreviewFromCursor();
}

bool UHousingPlacementComponent::ConfirmPlacement()
{
	AHousingAreaActor* HousingArea = ActiveArea.Get();
	if (State != EHousingPlacementState::Previewing || !HousingArea || !SelectedItem || !bCurrentPlacementValid)
	{
		return false;
	}

	AActor* PlacedActor = PreviewActor;
	if (!PlacedActor)
	{
		return false;
	}

	FHousingPlacementRecord Record = bMovingExistingPlacement
		? OriginalPlacementRecord
		: FHousingPlacementRecord();
	if (!bMovingExistingPlacement)
	{
		Record.PlacementId = FGuid::NewGuid();
	}
	Record.AreaId = HousingArea->GetHousingAreaId();
	Record.AnchorCell = CurrentAnchorCell;
	Record.ItemId = SelectedItem->GetPrimaryAssetId();
	Record.RotationQuarterTurns = CurrentRotationQuarterTurns;

	if (!HousingArea->RegisterPlacement(Record, CurrentOccupiedCells, PlacedActor, SelectedItem))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Housing placement confirmed: Data=%s, Actor=%s, Class=%s"),
		*GetNameSafe(SelectedItem),
		*GetNameSafe(PlacedActor),
		*GetNameSafe(PlacedActor->GetClass()));

	RestorePreviewActorState();
	PlacedActor->Tags.Remove(TEXT("HousingPlacementPreview"));
	PreviewActor = nullptr;
	SetComponentTickEnabled(false);
	ResetPlacementData();
	SetState(EHousingPlacementState::Browsing);
	return true;
}

void UHousingPlacementComponent::RecallPlacement()
{
	if (State != EHousingPlacementState::Previewing)
	{
		return;
	}

	DestroyPreview();
	SetState(EHousingPlacementState::Browsing);
}

void UHousingPlacementComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (State == EHousingPlacementState::Previewing)
	{
		UpdatePreviewFromCursor();
	}
}

void UHousingPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ExitHousing();
	Super::EndPlay(EndPlayReason);
}

void UHousingPlacementComponent::HandleExitInput()
{
	ExitHousing();
}

void UHousingPlacementComponent::HandleConfirmInput()
{
	ConfirmPlacement();
}

void UHousingPlacementComponent::HandleSelectPlacedPropInput()
{
	if (State != EHousingPlacementState::Browsing)
	{
		return;
	}

	ACPlayerController* PlayerController = GetPlayerController();
	FHitResult HitResult;
	if (PlayerController && PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		BeginMovePlacement(HitResult.GetActor());
	}
}

void UHousingPlacementComponent::HandleCancelSelectPlacedPropInput()
{
	if (State == EHousingPlacementState::Previewing && bMovingExistingPlacement)
	{
		CancelPlacement();
	}
}

bool UHousingPlacementComponent::BeginMovePlacement(AActor* PlacedActor)
{
	AHousingAreaActor* HousingArea = ActiveArea.Get();
	if (State != EHousingPlacementState::Browsing || !HousingArea || !IsValid(PlacedActor))
	{
		return false;
	}

	FHousingPlacementRecord PlacementRecord;
	TArray<FHousingCellCoord> OccupiedCells;
	UHousingItemDataAsset* ItemData = nullptr;
	if (!HousingArea->ReleasePlacement(PlacedActor, PlacementRecord, OccupiedCells, ItemData) || !ItemData)
	{
		return false;
	}

	PreviewActor = PlacedActor;
	SelectedItem = ItemData;
	bMovingExistingPlacement = true;
	OriginalPlacementRecord = PlacementRecord;
	OriginalOccupiedCells = MoveTemp(OccupiedCells);
	OriginalActorTransform = PlacedActor->GetActorTransform();
	CurrentRotationQuarterTurns = PlacementRecord.RotationQuarterTurns;
	bCurrentPlacementValid = false;
	bLastAppliedPreviewValidity = true;

	PlacedActor->Tags.AddUnique(TEXT("HousingPlacementPreview"));
	PreparePreviewActor();
	ApplyPreviewMaterial(false);
	SetState(EHousingPlacementState::Previewing);
	SetComponentTickEnabled(true);
	UpdatePreviewFromCursor();
	return true;
}

void UHousingPlacementComponent::UpdatePreviewFromCursor()
{
	ACPlayerController* PlayerController = GetPlayerController();
	AHousingAreaActor* HousingArea = ActiveArea.Get();
	if (!PlayerController || !HousingArea || !PreviewActor || !SelectedItem)
	{
		return;
	}

	FVector RayOrigin;
	FVector RayDirection;
	FHousingCellCoord Cell;
	FTransform CellTransform;
	const bool bFoundCell = PlayerController->DeprojectMousePositionToWorld(RayOrigin, RayDirection)
		&& HousingArea->FindCellFromRay(RayOrigin, RayDirection, Cell, CellTransform);

	TArray<FHousingCellCoord> CandidateCells;
	FTransform CandidateTransform;
	const bool bValid = bFoundCell && HousingArea->BuildPlacement(
		Cell,
		SelectedItem->Footprint,
		CurrentRotationQuarterTurns,
		CandidateCells,
		CandidateTransform);

	if (bFoundCell)
	{
		CurrentAnchorCell = Cell;
		CurrentPlacementTransform = CandidateTransform;
		CurrentOccupiedCells = MoveTemp(CandidateCells);
		FTransform PreviewTransform = CurrentPlacementTransform;
		PreviewTransform.AddToTranslation(PreviewTransform.TransformVectorNoScale(SelectedItem->PlacementOffset));
		PreviewActor->SetActorTransform(PreviewTransform);
		PreviewActor->SetActorHiddenInGame(false);
	}
	else
	{
		CurrentAnchorCell = FHousingCellCoord();
		CurrentOccupiedCells.Reset();
		PreviewActor->SetActorHiddenInGame(true);
	}

	bCurrentPlacementValid = bValid;
	ApplyPreviewMaterial(bValid);
}

AActor* UHousingPlacementComponent::SpawnPreviewActor(UHousingItemDataAsset* ItemData)
{
	UWorld* World = GetWorld();
	if (!World || !ItemData)
	{
		return nullptr;
	}

	if (UClass* PlacedClass = ItemData->PlacedActorClass.LoadSynchronous())
	{
		ACPlayerController* PlayerController = GetPlayerController();
		AActor* DeferredActor = World->SpawnActorDeferred<AActor>(
			PlacedClass,
			FTransform::Identity,
			GetOwner(),
			PlayerController ? PlayerController->GetPawn() : nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (DeferredActor)
		{
			DeferredActor->Tags.AddUnique(TEXT("HousingPlacementPreview"));
			return UGameplayStatics::FinishSpawningActor(DeferredActor, FTransform::Identity);
		}
	}

	UStaticMesh* PreviewMesh = ItemData->PreviewMesh.LoadSynchronous();
	if (!PreviewMesh)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("BeginPlacement failed: both PlacedActorClass and PreviewMesh are missing on %s."),
			*GetNameSafe(ItemData));
		return nullptr;
	}

	AStaticMeshActor* FallbackActor = World->SpawnActor<AStaticMeshActor>();
	if (!FallbackActor)
	{
		return nullptr;
	}

	FallbackActor->Tags.AddUnique(TEXT("HousingPlacementPreview"));
	UStaticMeshComponent* MeshComponent = FallbackActor->GetStaticMeshComponent();
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetStaticMesh(PreviewMesh);
	return FallbackActor;
}

void UHousingPlacementComponent::PreparePreviewActor()
{
	if (!PreviewActor)
	{
		return;
	}

	bPreviewActorTickEnabled = PreviewActor->IsActorTickEnabled();
	bPreviewActorCollisionEnabled = PreviewActor->GetActorEnableCollision();
	PreviewComponentTickStates.Reset();
	PreviewMobilityStates.Reset();
	PreviewCollisionStates.Reset();
	PreviewMeshMaterialStates.Reset();
	PreviewOriginalMaterials.Reset();

	TArray<UActorComponent*> ActorComponents;
	PreviewActor->GetComponents(ActorComponents);
	for (UActorComponent* Component : ActorComponents)
	{
		if (!Component)
		{
			continue;
		}

		PreviewComponentTickStates.Add(Component, Component->IsComponentTickEnabled());
		Component->SetComponentTickEnabled(false);

		if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component))
		{
			PreviewMobilityStates.Add(SceneComponent, SceneComponent->Mobility);
			SceneComponent->SetMobility(EComponentMobility::Movable);
		}

		if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component))
		{
			PreviewCollisionStates.Add(PrimitiveComponent, PrimitiveComponent->GetCollisionEnabled());
			PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component))
		{
			FHousingPreviewMeshMaterialState MaterialState;
			MaterialState.Component = MeshComponent;
			MaterialState.FirstMaterialIndex = PreviewOriginalMaterials.Num();
			MaterialState.NumMaterials = MeshComponent->GetNumMaterials();
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialState.NumMaterials; ++MaterialIndex)
			{
				PreviewOriginalMaterials.Add(MeshComponent->GetMaterial(MaterialIndex));
			}
			PreviewMeshMaterialStates.Add(MaterialState);
		}
	}

	PreviewActor->SetActorTickEnabled(false);
	PreviewActor->SetActorEnableCollision(false);
}

void UHousingPlacementComponent::RestorePreviewActorState()
{
	if (!PreviewActor)
	{
		return;
	}

	for (const FHousingPreviewMeshMaterialState& MaterialState : PreviewMeshMaterialStates)
	{
		if (UMeshComponent* MeshComponent = MaterialState.Component.Get())
		{
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialState.NumMaterials; ++MaterialIndex)
			{
				const int32 CachedIndex = MaterialState.FirstMaterialIndex + MaterialIndex;
				MeshComponent->SetMaterial(MaterialIndex, PreviewOriginalMaterials.IsValidIndex(CachedIndex)
					? PreviewOriginalMaterials[CachedIndex]
					: nullptr);
			}
		}
	}

	for (const TPair<TWeakObjectPtr<UPrimitiveComponent>, TEnumAsByte<ECollisionEnabled::Type>>& Pair : PreviewCollisionStates)
	{
		if (UPrimitiveComponent* Component = Pair.Key.Get())
		{
			Component->SetCollisionEnabled(Pair.Value);
		}
	}

	for (const TPair<TWeakObjectPtr<USceneComponent>, TEnumAsByte<EComponentMobility::Type>>& Pair : PreviewMobilityStates)
	{
		if (USceneComponent* Component = Pair.Key.Get())
		{
			Component->SetMobility(Pair.Value);
		}
	}

	for (const TPair<TWeakObjectPtr<UActorComponent>, bool>& Pair : PreviewComponentTickStates)
	{
		if (UActorComponent* Component = Pair.Key.Get())
		{
			Component->SetComponentTickEnabled(Pair.Value);
		}
	}

	PreviewActor->SetActorEnableCollision(bPreviewActorCollisionEnabled);
	PreviewActor->SetActorTickEnabled(bPreviewActorTickEnabled);
	PreviewComponentTickStates.Reset();
	PreviewMobilityStates.Reset();
	PreviewCollisionStates.Reset();
	PreviewMeshMaterialStates.Reset();
	PreviewOriginalMaterials.Reset();
}

void UHousingPlacementComponent::DestroyPreview()
{
	SetComponentTickEnabled(false);
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	ResetPlacementData();
}

void UHousingPlacementComponent::ResetPlacementData()
{
	PreviewComponentTickStates.Reset();
	PreviewMobilityStates.Reset();
	PreviewCollisionStates.Reset();
	PreviewMeshMaterialStates.Reset();
	PreviewOriginalMaterials.Reset();
	SelectedItem = nullptr;
	CurrentAnchorCell = FHousingCellCoord();
	CurrentPlacementTransform = FTransform::Identity;
	CurrentOccupiedCells.Reset();
	CurrentRotationQuarterTurns = 0;
	bCurrentPlacementValid = false;
	bMovingExistingPlacement = false;
	OriginalPlacementRecord = FHousingPlacementRecord();
	OriginalOccupiedCells.Reset();
	OriginalActorTransform = FTransform::Identity;
}

void UHousingPlacementComponent::ApplyPreviewMaterial(bool bPlacementValid)
{
	if (!PreviewActor || bLastAppliedPreviewValidity == bPlacementValid)
	{
		return;
	}

	bLastAppliedPreviewValidity = bPlacementValid;
	UMaterialInterface* Material = bPlacementValid ? ValidPreviewMaterial : InvalidPreviewMaterial;
	if (!Material)
	{
		return;
	}

	for (const FHousingPreviewMeshMaterialState& MaterialState : PreviewMeshMaterialStates)
	{
		if (UMeshComponent* MeshComponent = MaterialState.Component.Get())
		{
			for (int32 MaterialIndex = 0; MaterialIndex < MaterialState.NumMaterials; ++MaterialIndex)
			{
				MeshComponent->SetMaterial(MaterialIndex, Material);
			}
		}
	}
}

void UHousingPlacementComponent::SetState(EHousingPlacementState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	OnStateChanged.Broadcast(State);
}

ACPlayerController* UHousingPlacementComponent::GetPlayerController() const
{
	return Cast<ACPlayerController>(GetOwner());
}

void UHousingPlacementComponent::AddHousingInputMapping()
{
	ACPlayerController* PlayerController = GetPlayerController();
	if (bInputMappingAdded || !HousingInputMappingContext || !PlayerController || !PlayerController->GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(HousingInputMappingContext, HousingInputPriority);
		bInputMappingAdded = true;
	}
}

void UHousingPlacementComponent::RemoveHousingInputMapping()
{
	ACPlayerController* PlayerController = GetPlayerController();
	if (!bInputMappingAdded || !PlayerController || !PlayerController->GetLocalPlayer())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->RemoveMappingContext(HousingInputMappingContext);
	}

	bInputMappingAdded = false;
}
