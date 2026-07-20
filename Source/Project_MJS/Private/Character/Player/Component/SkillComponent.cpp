#include "Character/Player/Component/SkillComponent.h"

#include "Animation/AnimInstance.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Character/SharedComponent/StaminaComponent.h"
#include "Cinematic/CinematicActionComponent.h"
#include "Cinematic/CinematicTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

USkillComponent::USkillComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USkillComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* Owner = GetOwner())
	{
		CinematicActionComp = Owner->FindComponentByClass<UCinematicActionComponent>();
		StaminaComponent = Owner->FindComponentByClass<UStaminaComponent>();
	}
}

void USkillComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SkillState == ESkillActivationState::Cinematic && !IsSkillCinematicPlaying())
	{
		if (ActiveSkillMontage)
		{
			SkillState = ESkillActivationState::Active;
			return;
		}

		FinishSkill();
	}
}

bool USkillComponent::ActivateSkill(USkillDataAsset* SkillData)
{
	if (!SkillData || !GetWorld() || !CinematicActionComp)
	{
		UE_LOG(LogTemp, Error, TEXT("SkillComp : SkillData나 Getworld, CinematicActionComp가 할당되지 않았습니다."));
		return false;
	}

	if (!IsValid(StaminaComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("Skill activation failed: StaminaComponent is missing. Owner=%s"), *GetNameSafe(GetOwner()));
		return false;
	}
	
	if (SkillState != ESkillActivationState::Idle || IsSkillCinematicPlaying())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComp : 이미 스킬이 플레이 중이거나, ESkillActivationState::Idle 상태가 아닙니다."));
		return false;
	}

	const FSkillExecutionSettings& ExecutionSettings = SkillData->ExecutionSettings;
	const FSkillCinematicSettings& CinematicSettings = SkillData->CinematicSettings;

	if (!StaminaComponent->CanConsumeStamina(ExecutionSettings.StaminaCost))
	{
		UE_LOG(LogTemp, Log, TEXT("Skill activation rejected: insufficient stamina. Skill=%s Current=%.1f Cost=%.1f"),
			*GetNameSafe(SkillData), StaminaComponent->GetCurrentStamina(), ExecutionSettings.StaminaCost.StaminaCost);
		return false;
	}
	
	ActiveSkillData = SkillData;
	SkillState = ESkillActivationState::Startup;
	SetComponentTickEnabled(true);
	StopActiveMontages();

	bool bPlayedMontage = false;
	if (ExecutionSettings.MontageToPlay)
	{
		AActor* Owner = GetOwner();
		if (ACharacter* Char = Cast<ACharacter>(Owner))
		{
			UAnimInstance* AnimInstance = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr;
			if (IsValid(AnimInstance))
			{
				const float Duration = AnimInstance->Montage_Play(ExecutionSettings.MontageToPlay, 1.0f);
				if (Duration > 0.0f)
				{
					FOnMontageEnded MontageEndedDelegate;
					MontageEndedDelegate.BindUObject(this, &USkillComponent::OnSkillMontageEnded);
					AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, ExecutionSettings.MontageToPlay);

					ActiveSkillMontage = ExecutionSettings.MontageToPlay;
					SkillState = ESkillActivationState::Active;
					bPlayedMontage = true;
				}
			}
		}
	}
	
	bool bPlayedCinematic = false;
	if (CinematicSettings.bUseCinematic && CinematicSettings.CinematicSequence)
	{
		PlaySkillCinematic(SkillData);
		bPlayedCinematic = IsSkillCinematicPlaying();
	}

	if (!bPlayedMontage && !bPlayedCinematic)
	{
		FinishSkill();
		UE_LOG(LogTemp, Warning, TEXT("SkillComp : 플레이 할 몽타주나 시네마틱이 할당되지 않았습니다."));

		return false;
	}

	if (!StaminaComponent->ConsumeStamina(ExecutionSettings.StaminaCost))
	{
		StopActiveMontages();
		FinishSkill();
		return false;
	}
	
	return true;
}

void USkillComponent::RequestSkillEvent(FName EventName)
{
	if (EventName.IsNone() || !ActiveSkillData)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Skill event requested. Skill=%s Event=%s"), *GetNameSafe(ActiveSkillData.Get()), *EventName.ToString());

	if (EventName == TEXT("Finish"))
	{
		FinishSkill();
	}
}

bool USkillComponent::IsSkillCinematicPlaying() const
{
	return ActiveSkillData != nullptr && CinematicActionComp && CinematicActionComp->IsCinematicPlaying();
}

void USkillComponent::PlaySkillCinematic(USkillDataAsset* SkillData)
{
	if (!SkillData || !SkillData->CinematicSettings.CinematicSequence || !CinematicActionComp)
	{
		return;
	}
	
	FCinematicPlaybackRequest Request = BuildSkillCinematicRequest(SkillData);
	if (CinematicActionComp->PlayCinematicRequest(Request))
	{
		SkillState = ESkillActivationState::Cinematic;
	}
}

FCinematicPlaybackRequest USkillComponent::BuildSkillCinematicRequest(const USkillDataAsset* SkillData) const
{
	FCinematicPlaybackRequest Request;
	if (!SkillData)
	{
		return Request;
	}

	AActor* Owner = GetOwner();
	AActor* BestTarget = ResolveBestSkillTarget(SkillData);
	AActor* SubjectActor = BestTarget ? BestTarget : Owner;
	const FSkillCinematicSettings& CinematicSettings = SkillData->CinematicSettings;

	Request.Sequence = CinematicSettings.CinematicSequence;
	Request.InstigatorActor = Owner;
	Request.SubjectActor = SubjectActor;
	Request.AnchorActor = Owner;
	Request.bRestoreViewTarget = CinematicSettings.bRestoreViewTarget;
	Request.BlendOutTime = CinematicSettings.CinematicBlendOutTime;
	Request.ParticipantScope = CinematicSettings.ParticipantScope;
	Request.bStopPreviousCinematic = CinematicSettings.bStopPreviousCinematic;
	Request.AnchorMode = CinematicSettings.AnchorMode;
	Request.RotationSource = CinematicSettings.RotationSource;
	Request.AnchorSocketName = CinematicSettings.AnchorSocketName;
	Request.TargetSocketName = CinematicSettings.TargetSocketName;
	Request.RelativeTransform = CinematicSettings.RelativeTransform;
	Request.bUseYawOnly = CinematicSettings.bUseYawOnly;

	if (SkillData->Type == ESkillType::Ultimate && CinematicSettings.bUseUltimateCinematicDefaults)
	{
		Request.ParticipantScope = ECinematicParticipantScope::AllInWorld;
		Request.bStopPreviousCinematic = true;
		Request.AnchorMode = BestTarget ? ECinematicAnchorMode::InstigatorToSubject : ECinematicAnchorMode::SubjectActor;
	}

	if (CinematicSettings.bBindBestTarget && BestTarget && !CinematicSettings.TargetBindingTag.IsNone())
	{
		AddActorBinding(Request, CinematicSettings.TargetBindingTag, BestTarget, CinematicSettings.bAllowTargetBindingFromAsset);
		Request.AdditionalParticipants.AddUnique(BestTarget);
	}

	return Request;
}

AActor* USkillComponent::ResolveBestSkillTarget(const USkillDataAsset* SkillData) const
{
	if (!SkillData || !SkillData->CinematicSettings.bBindBestTarget)
	{
		return nullptr;
	}

	const AActor* Owner = GetOwner();
	const UTargetingComponent* TargetingComponent = Owner ? Owner->FindComponentByClass<UTargetingComponent>() : nullptr;
	return TargetingComponent ? TargetingComponent->GetBestAttackTarget() : nullptr;
}

void USkillComponent::AddActorBinding(FCinematicPlaybackRequest& Request, FName BindingTag, AActor* Actor, bool bAllowBindingsFromAsset) const
{
	if (BindingTag.IsNone() || !IsValid(Actor))
	{
		return;
	}

	FCinematicBindingOverride BindingOverride;
	BindingOverride.BindingTag = BindingTag;
	BindingOverride.Actors.Add(Actor);
	BindingOverride.bAllowBindingsFromAsset = bAllowBindingsFromAsset;
	Request.BindingOverrides.Add(BindingOverride);
}

void USkillComponent::OnSkillMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != ActiveSkillMontage)
	{
		return;
	}

	ActiveSkillMontage = nullptr;

	if (IsSkillCinematicPlaying())
	{
		SkillState = ESkillActivationState::Cinematic;
		return;
	}

	FinishSkill();
}

void USkillComponent::FinishSkill()
{
	ActiveSkillMontage = nullptr;
	ActiveSkillData = nullptr;
	SkillState = ESkillActivationState::Idle;
	SetComponentTickEnabled(false);
}

void USkillComponent::StopActiveMontages()
{
	AActor* Owner = GetOwner();
	if (!Owner) 
	{
		return;
	}

	ACharacter* Char = Cast<ACharacter>(Owner);
	if (!Char || !Char->GetMesh())
	{
		return;
	}
	
	UAnimInstance* AnimInst = Char->GetMesh()->GetAnimInstance();
	if (!AnimInst)
	{
		return;
	}
	
	AnimInst->Montage_Stop(0.1f);
}
