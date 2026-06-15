#include "Character/Player/Component/AttackComponent.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/Player/Data/ComboAttackDataAsset.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 평소에는 연산하지 않도록 꺼둡니다.
	PrimaryComponentTick.bStartWithTickEnabled = false; 
}

void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsWeaponAttacking)
	{
		CheckWeaponTrace();
	}
	
	if (bIsKickAttacking)
	{
		CheckKickTrace();
	}
}

void UAttackComponent::RequestAttack()
{
	if (bIsAttacking)
	{
		if (HasNextCombo())
		{
			bComboQueued = true;
			UE_LOG(LogTemp, Log, TEXT("RequestAttack: queued next combo. WindowOpen=%s CurrentComboIndex=%d"), bCanQueueCombo ? TEXT("true") : TEXT("false"), CurrentComboIndex);
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("RequestAttack rejected: already attacking and next combo does not exist. CurrentComboIndex=%d"), CurrentComboIndex);
		return;
	}

	PlayCombo(0);
}

void UAttackComponent::SetComboWindowOpen(bool bOpen)
{
	bCanQueueCombo = bOpen;
	UE_LOG(LogTemp, Log, TEXT("ComboWindow %s. CurrentComboIndex=%d Queued=%s"), bCanQueueCombo ? TEXT("opened") : TEXT("closed"), CurrentComboIndex, bComboQueued ? TEXT("true") : TEXT("false"));

	if (!bCanQueueCombo && bComboQueued)
	{
		PlayQueuedCombo();
	}
}

bool UAttackComponent::PlayCombo(int32 ComboIndex)
{
	if (!DA_BasicComboAttack || !DA_BasicComboAttack->ComboEntries.IsValidIndex(ComboIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: combo data is missing or index is invalid. DataAsset=%s Index=%d"), *GetNameSafe(DA_BasicComboAttack), ComboIndex);
		return false;
	}

	const FComboAttackEntry& ComboEntry = DA_BasicComboAttack->ComboEntries[ComboIndex];
	if (!ComboEntry.Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Montage is missing. DataAsset=%s Index=%d"), *GetNameSafe(DA_BasicComboAttack), ComboIndex);
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Owner is not ACharacter."));
		return false;
	}

	USkeletalMeshComponent* MeshComponent = OwnerCharacter->GetMesh();
	UAnimInstance* AnimInstance = MeshComponent ? MeshComponent->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: AnimInstance is missing. Character=%s"), *GetNameSafe(OwnerCharacter));
		return false;
	}

	if (ACPlayerCharacter* PlayerCharacter = Cast<ACPlayerCharacter>(OwnerCharacter))
	{
		FVector AttackDirection;
		if (PlayerCharacter->GetLastMoveWorldDirection(AttackDirection))
		{
			OwnerCharacter->SetActorRotation(AttackDirection.Rotation());
		}
	}

	const float Duration = AnimInstance->Montage_Play(ComboEntry.Montage);
	if (Duration <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayCombo failed: Montage_Play returned 0. Check AnimBP slot setup. Montage=%s"), *GetNameSafe(ComboEntry.Montage));
		return false;
	}

	if (ComboEntry.SectionName != NAME_None)
	{
		AnimInstance->Montage_JumpToSection(ComboEntry.SectionName, ComboEntry.Montage);
	}

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UAttackComponent::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ComboEntry.Montage);

	CurrentComboIndex = ComboIndex;
	ActiveMontage = ComboEntry.Montage;
	bIsAttacking = true;
	bCanQueueCombo = false;
	bComboQueued = false;

	UE_LOG(LogTemp, Log, TEXT("PlayCombo succeeded: Index=%d Montage=%s Section=%s Duration=%.2f"), ComboIndex, *GetNameSafe(ComboEntry.Montage), *ComboEntry.SectionName.ToString(), Duration);
	return true;
}

void UAttackComponent::PlayQueuedCombo()
{
	const int32 NextComboIndex = CurrentComboIndex + 1;
	if (!PlayCombo(NextComboIndex))
	{
		bComboQueued = false;
	}
}

bool UAttackComponent::HasNextCombo() const
{
	return DA_BasicComboAttack && DA_BasicComboAttack->ComboEntries.IsValidIndex(CurrentComboIndex + 1);
}

void UAttackComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveMontage)
	{
		return;
	}

	bIsAttacking = false;
	bCanQueueCombo = false;
	bComboQueued = false;
	CurrentComboIndex = INDEX_NONE;
	ActiveMontage = nullptr;
}

// =============== 공격 판정 용 함수 구현부 ==================

// 카타나, 대검 등 무기전용
void UAttackComponent::StartWeaponAttack()
{
	HitActors.Empty();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		LastWeaponStartPos = OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("WeaponAttach_StartSocket"));
		LastWeaponEndPos = OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("WeaponAttach_EndSocket"));
	}

	bIsWeaponAttacking = true;
	SetComponentTickEnabled(true);
}

void UAttackComponent::EndWeaponAttack()
{
	bIsWeaponAttacking = false;
	if (!bIsKickAttacking) 
	{
		SetComponentTickEnabled(false); 
	}
}

void UAttackComponent::CheckWeaponTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;

	FVector CurrentWeaponStartPos = OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("WeaponAttach_StartSocket"));
	FVector CurrentWeaponEndPos = OwnerCharacter->GetMesh()->GetSocketLocation(TEXT("WeaponAttach_EndSocket"));
	
	int32 NumSegments = 8;
	float SweepRadius = 15.0f;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	for (int32 i = 0; i <= NumSegments; ++i)
	{
		float Alpha = (float)i / NumSegments;
		FVector SegmentLastPos = FMath::Lerp(LastWeaponStartPos, LastWeaponEndPos, Alpha);
		FVector SegmentCurrPos = FMath::Lerp(CurrentWeaponStartPos, CurrentWeaponEndPos, Alpha);

		FHitResult HitResult;
		bool bHit = GetWorld()->SweepSingleByChannel(
			HitResult, SegmentLastPos, SegmentCurrPos, FQuat::Identity, 
			ECC_GameTraceChannel1, FCollisionShape::MakeSphere(SweepRadius), Params
		);
		
		DrawDebugAttackShape(SegmentLastPos, SegmentCurrPos, SweepRadius, bHit);

		if (bHit && HitResult.GetActor())
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActors.Contains(HitActor))
			{
				HitActors.Add(HitActor); 
				
				APawn* OwnerPawn = Cast<APawn>(GetOwner());
				AController* InstigatorController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
				FVector ShotDirection = (HitActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();

				UGameplayStatics::ApplyPointDamage(
					HitActor, 30.0f, ShotDirection, HitResult, InstigatorController, GetOwner(), UDamageType::StaticClass()
				);
			}
		}
	}

	LastWeaponStartPos = CurrentWeaponStartPos;
	LastWeaponEndPos = CurrentWeaponEndPos;
}

// 발차기 전용
void UAttackComponent::StartKickAttack(FName SocketName)
{
	HitActors.Empty();
	CurrentKickSocket = SocketName;
	
	bIsKickAttacking = true;
	SetComponentTickEnabled(true);
}

void UAttackComponent::EndKickAttack()
{
	bIsKickAttacking = false;
	if (!bIsWeaponAttacking)
	{
		SetComponentTickEnabled(false);
	}
}

void UAttackComponent::CheckKickTrace()
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter) return;
	
	FVector KickLocation = OwnerCharacter->GetMesh()->GetSocketLocation(CurrentKickSocket);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	// 발 주변을 감싸는 구체 판정
	float KickRadius = 30.0f;
	
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult, 
		KickLocation, // Start
		KickLocation, // End
		FQuat::Identity,
		ECC_GameTraceChannel1, 
		FCollisionShape::MakeSphere(KickRadius), 
		Params
	);
	
	DrawDebugAttackShape(KickLocation, KickLocation, KickRadius, bHit);

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		if (!HitActors.Contains(HitActor))
		{
			HitActors.Add(HitActor);
			
			AController* InstigatorController = OwnerCharacter->GetController();
			
			FVector ShotDirection = (HitActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();

			UGameplayStatics::ApplyPointDamage(
				HitActor, 
				20.0f, 
				ShotDirection, 
				HitResult, 
				InstigatorController, 
				GetOwner(), 
				UDamageType::StaticClass()
			);
		}
	}
}

void UAttackComponent::DrawDebugAttackShape(const FVector& StartPos, const FVector& EndPos, float Radius, bool bHit)
{
#if ENABLE_DRAW_DEBUG
	if (!bShowDebugShape) return;

	FColor DrawColor = bHit ? FColor::Red : FColor::Green;
	float LifeTime = 0.5f; 

	if (StartPos == EndPos)
	{
		DrawDebugSphere(GetWorld(), StartPos, Radius, 16, DrawColor, false, LifeTime);
	}
	else
	{
		FVector Center = (StartPos + EndPos) * 0.5f;
		float HalfHeight = (EndPos - StartPos).Size() * 0.5f + Radius;
		FQuat Rotation = FRotationMatrix::MakeFromZ(EndPos - StartPos).ToQuat();
		
		DrawDebugCapsule(GetWorld(), Center, HalfHeight, Radius, Rotation, DrawColor, false, LifeTime);
	}
#endif
}