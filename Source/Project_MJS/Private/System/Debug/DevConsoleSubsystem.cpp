#include "System/Debug/DevConsoleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Character/Enemy/EnemyCharacter.h"
#include "Character/Player/CPlayerController.h"
#include "Character/Player/CPlayerCharacter.h"
#include "Character/Player/Component/AttackComponent.h"
#include "Character/Player/Component/DodgeComponent.h"
#include "Character/Player/Component/TargetingComponent.h"
#include "Character/SharedComponent/HealthComponent.h"
#include "Cinematic/CinematicDirectorSubsystem.h"
#include "Cinematic/CinematicInputLockSubsystem.h"
#include "Camera/CameraRigActor.h"
#include "Level/TeleportZone.h"
#include "System/Audio/SoundManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if !UE_BUILD_SHIPPING

void UDevConsoleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterCoreCommands();
}

void UDevConsoleSubsystem::Deinitialize()
{
	// 필요 시 정리 로직 추가
	Super::Deinitialize();
}

FString UDevConsoleSubsystem::ExecuteCommand(const FString& RawInput)
{
	FString Result = FDevCommandDispatcher::ExecuteCommand(GetWorld(), RawInput);

	// 최근 로그에 기록 (UI에서 사용 가능하도록)
	if (!Result.IsEmpty())
	{
		RecentLogs.Add(FString::Printf(TEXT("[%s] %s"), *FDateTime::Now().ToString(), *RawInput));
		RecentLogs.Add(Result);
	}

	return Result;
}

void UDevConsoleSubsystem::RegisterCoreCommands()
{
	FDevCommandDispatcher::RegisterCommand(
		TEXT("help"),
		TEXT("System"),
		TEXT("Show usage and available command categories."),
		[](UWorld*, const TArray<FString>&) -> FString
		{
			return TEXT("Developer Console Help:\n"
			           "- help: Show this message.\n"
			           "- list [category]: List commands. Example: 'list Cinematic'.\n"
			           "Type a command directly to execute it.");
		});

	FDevCommandDispatcher::RegisterCommand(
		TEXT("list"),
		TEXT("System"),
		TEXT("List available commands, optionally filtered by category."),
		[](UWorld*, const TArray<FString>& Args) -> FString
		{
			const auto& Commands = FDevCommandDispatcher::GetCommands();

			FString Filter;
			if (!Args.IsEmpty())
			{
				Filter = Args[0];
			}

			TArray<FString> Lines;
			Lines.Add(TEXT("Available commands:"));

			for (const auto& KV : Commands)
			{
				const FDevCommandEntry& E = KV.Value;
				if (!Filter.IsEmpty() && !E.Category.Contains(Filter, ESearchCase::IgnoreCase))
				{
					continue;
				}

				Lines.Add(FString::Printf(TEXT("  %s [%s] - %s"), *E.Name, *E.Category, *E.Description));
			}

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Cinematic Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("cinematic.status"),
		TEXT("Cinematic"),
		TEXT("Show current cinematic playback status."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			UCinematicDirectorSubsystem* Director = World->GetSubsystem<UCinematicDirectorSubsystem>();
			if (!Director) return TEXT("Cinematic Director Subsystem not found.");

			return Director->GetCinematicStatusSummary();
		});

	FDevCommandDispatcher::RegisterCommand(
		TEXT("cinematic.stop"),
		TEXT("Cinematic"),
		TEXT("Stop current cinematic playback."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			UCinematicDirectorSubsystem* Director = World->GetSubsystem<UCinematicDirectorSubsystem>();
			if (!Director) return TEXT("Cinematic Director Subsystem not found.");

			Director->StopCinematic();
			return TEXT("Cinematic stopped.");
		});

	// ===== Player Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("player.status"),
		TEXT("Player"),
		TEXT("Show player status (HP, position, input lock)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			TArray<FString> Lines;
			Lines.Add(TEXT("Player Status:"));
			Lines.Add(FString::Printf(TEXT("  Location: %s"), *PlayerChar->GetActorLocation().ToString()));
			Lines.Add(FString::Printf(TEXT("  Velocity: %s"), *PlayerChar->GetVelocity().ToString()));

			if (UHealthComponent* HealthComp = PlayerChar->GetHealthComponent())
			{
				Lines.Add(FString::Printf(TEXT("  HP: %.1f / %.1f"), HealthComp->GetCurrentHealth(), HealthComp->GetMaxHealth()));
			}

			UCinematicInputLockSubsystem* InputLockSub = World->GetSubsystem<UCinematicInputLockSubsystem>();
			if (InputLockSub)
			{
				Lines.Add(FString::Printf(TEXT("  MoveLocked: %s"), InputLockSub->IsMoveInputLocked(PC) ? TEXT("Yes") : TEXT("No")));
				Lines.Add(FString::Printf(TEXT("  LookLocked: %s"), InputLockSub->IsLookInputLocked(PC) ? TEXT("Yes") : TEXT("No")));
				Lines.Add(FString::Printf(TEXT("  GameplayLocked: %s"), InputLockSub->IsGameplayInputLocked(PC) ? TEXT("Yes") : TEXT("No")));
			}

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Combat Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("combat.status"),
		TEXT("Combat"),
		TEXT("Show combat status (attack, combo, dodge)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			TArray<FString> Lines;
			Lines.Add(TEXT("Combat Status:"));

			UAttackComponent* AttackComp = PlayerChar->GetAttackComponent();
			if (AttackComp)
			{
				Lines.Add(FString::Printf(TEXT("  Attacking: %s"), AttackComp->IsAttacking() ? TEXT("Yes") : TEXT("No")));
				Lines.Add(FString::Printf(TEXT("  ComboIndex: %d"), AttackComp->GetCurrentComboIndex()));
				Lines.Add(FString::Printf(TEXT("  CanQueueCombo: %s"), AttackComp->CanQueueCombo() ? TEXT("Yes") : TEXT("No")));
				Lines.Add(FString::Printf(TEXT("  ComboQueued: %s"), AttackComp->IsComboQueued() ? TEXT("Yes") : TEXT("No")));
			}

			UDodgeComponent* DodgeComp = PlayerChar->GetDodgeComponent();
			if (DodgeComp)
			{
				Lines.Add(FString::Printf(TEXT("  Dodging: %s"), DodgeComp->IsDodging() ? TEXT("Yes") : TEXT("No")));
			}

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Camera Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("camera.status"),
		TEXT("Camera"),
		TEXT("Show camera status (ViewTarget, CameraRig)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerController* PlayerCtrl = Cast<ACPlayerController>(PC);

			TArray<FString> Lines;
			Lines.Add(TEXT("Camera Status:"));
			Lines.Add(FString::Printf(TEXT("  ViewTarget: %s"), *GetNameSafe(PC->GetViewTarget())));

			if (PlayerCtrl && PlayerCtrl->GetCameraRig())
			{
				Lines.Add(TEXT("  CameraRig: Exists"));
				Lines.Add(FString::Printf(TEXT("  CameraTarget: %s"), *GetNameSafe(PlayerCtrl->GetCameraRig()->GetCurrentTarget())));
				Lines.Add(FString::Printf(TEXT("  FocusTarget: %s"), *GetNameSafe(PlayerCtrl->GetCameraRig()->GetFocusTarget())));
			}
			else
			{
				Lines.Add(TEXT("  CameraRig: None"));
			}

			return FString::Join(Lines, TEXT("\n"));
		});

	FDevCommandDispatcher::RegisterCommand(
		TEXT("camera.reset_viewtarget"),
		TEXT("Camera"),
		TEXT("Reset ViewTarget to player character."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			PC->SetViewTargetWithBlend(PlayerChar, 0.1f);
			return TEXT("ViewTarget reset to player character.");
		});

	// ===== Targeting Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("targeting.status"),
		TEXT("Targeting"),
		TEXT("Show targeting status (hard/soft targets, candidates)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			UTargetingComponent* TargetComp = PlayerChar->GetTargetingComponent();
			if (!TargetComp) return TEXT("Targeting component not found.");

			TArray<FString> Lines;
			Lines.Add(TEXT("Targeting Status:"));
			Lines.Add(FString::Printf(TEXT("  HardTarget: %s"), *GetNameSafe(TargetComp->GetHardTarget())));
			Lines.Add(FString::Printf(TEXT("  AutoTarget: %s"), *GetNameSafe(TargetComp->GetAutoTarget())));
			Lines.Add(FString::Printf(TEXT("  CandidatesCount: %d"), TargetComp->GetCandidateTargetsCount()));
			Lines.Add(FString::Printf(TEXT("  RangedAiming: %s"), TargetComp->IsRangedHardTargetAiming() ? TEXT("Yes") : TEXT("No")));

			return FString::Join(Lines, TEXT("\n"));
		});

	FDevCommandDispatcher::RegisterCommand(
		TEXT("targeting.clear"),
		TEXT("Targeting"),
		TEXT("Clear hard target."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			UTargetingComponent* TargetComp = PlayerChar->GetTargetingComponent();
			if (!TargetComp) return TEXT("Targeting component not found.");

			TargetComp->ClearHardTarget();
			return TEXT("Hard target cleared.");
		});

	// ===== AI Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("ai.status"),
		TEXT("AI"),
		TEXT("Show AI status (active enemies count)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			int32 EnemyCount = 0;
			for (TActorIterator<AEnemyCharacter> It(World); It; ++It)
			{
				++EnemyCount;
			}

			TArray<FString> Lines;
			Lines.Add(TEXT("AI Status:"));
			Lines.Add(FString::Printf(TEXT("  ActiveEnemies: %d"), EnemyCount));

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Level Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("level.triggers"),
		TEXT("Level"),
		TEXT("List triggers and teleport zones in current level."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			TArray<FString> Lines;
			Lines.Add(TEXT("Level Triggers:"));

			int32 TeleportZoneCount = 0;
			for (TActorIterator<ATeleportZone> It(World); It; ++It)
			{
				++TeleportZoneCount;
			}
			Lines.Add(FString::Printf(TEXT("  TeleportZones: %d"), TeleportZoneCount));

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Audio Commands =====
	FDevCommandDispatcher::RegisterCommand(
		TEXT("audio.status"),
		TEXT("Audio"),
		TEXT("Show audio status (BGM state)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
			if (!GI) return TEXT("Error: GameInstance is missing.");

			USoundManagerSubsystem* SoundMgr = GI->GetSubsystem<USoundManagerSubsystem>();
			if (!SoundMgr) return TEXT("Sound Manager Subsystem not found.");

			TArray<FString> Lines;
			Lines.Add(TEXT("Audio Status:"));
			Lines.Add(FString::Printf(TEXT("  BGMState: %d"), static_cast<int32>(SoundMgr->GetBGMState())));

			return FString::Join(Lines, TEXT("\n"));
		});

	// ===== Phase 1: Test Loop Recovery Commands =====

	// level.restart
	FDevCommandDispatcher::RegisterCommand(
		TEXT("level.restart"),
		TEXT("Level"),
		TEXT("Restart current level (useful for test loop)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
			if (LevelName.IsEmpty())
			{
				return TEXT("Error: Could not read current map name.");
			}

			UGameplayStatics::OpenLevel(World, FName(*LevelName));
			return FString::Printf(TEXT("Restarting level '%s'..."), *LevelName);
		});

	// player.reset_state
	FDevCommandDispatcher::RegisterCommand(
		TEXT("player.reset_state"),
		TEXT("Player"),
		TEXT("Reset player state (HP, input locks, combat states)."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			PlayerChar->ResetState();
			return TEXT("Player state reset: HP max, input unlocked, combat states cleared.");
		});

	// player.heal_full
	FDevCommandDispatcher::RegisterCommand(
		TEXT("player.heal_full"),
		TEXT("Player"),
		TEXT("Set player HP to maximum."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			PlayerChar->HealFull();
			return TEXT("Player HP set to maximum.");
		});

	// player.revive
	FDevCommandDispatcher::RegisterCommand(
		TEXT("player.revive"),
		TEXT("Player"),
		TEXT("Revive dead player and reset state."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return TEXT("No player controller found.");

			ACPlayerCharacter* PlayerChar = Cast<ACPlayerCharacter>(PC->GetPawn());
			if (!PlayerChar) return TEXT("Player character not found or invalid type.");

			PlayerChar->Revive();
			return TEXT("Player revived and state reset.");
		});

	// ai.respawn_all
	FDevCommandDispatcher::RegisterCommand(
		TEXT("ai.respawn_all"),
		TEXT("AI"),
		TEXT("Respawn all dead enemies in current level."),
		[](UWorld* World, const TArray<FString>&) -> FString
		{
			if (!World) return TEXT("Error: World is missing.");

			int32 RespawnedCount = 0;

			TArray<AActor*> DeadEnemies;
			for (TActorIterator<AEnemyCharacter> It(World); It; ++It)
			{
				AEnemyCharacter* Enemy = *It;
				if (!Enemy->GetHealthComponent() || !Enemy->GetHealthComponent()->IsAlive())
				{
					DeadEnemies.Add(Enemy);
				}
			}

			for (AActor* Dead : DeadEnemies)
			{
				AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Dead);
				if (!Enemy) continue;

				const FTransform SpawnTransform = Enemy->GetActorTransform();
				UClass* EnemyClass = Enemy->GetClass();
				Enemy->Destroy();

				if (!EnemyClass)
				{
					continue;
				}

				AEnemyCharacter* NewEnemy = World->SpawnActor<AEnemyCharacter>(
					EnemyClass,
					SpawnTransform,
					FActorSpawnParameters());

				if (NewEnemy && NewEnemy->GetHealthComponent())
				{
					NewEnemy->GetHealthComponent()->Heal(NewEnemy->GetHealthComponent()->GetMaxHealth());
					++RespawnedCount;
				}
			}

			return FString::Printf(TEXT("ai.respawn_all: Respawned %d enemies."), RespawnedCount);
		});
}

#endif // !UE_BUILD_SHIPPING
