// Copyright Epic Games, Inc. All Rights Reserved.

#include "EndlessRunnerGameMode.h"
#include "TrackGenerator.h"
#include "GameplayManager.h"
#include "CurrencyManager.h"
#include "RabbitCharacter.h"
#include "RabbitJumpComponent.h"
#include "RabbitMovementComponent.h"
#include "SpawnManager.h"
#include "TrackPiece.h"
#include "../UI/EndlessRunnerHUD.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "TimerManager.h"
#include "CollectibleCoin.h"
#include "MultiCollectible.h"
#include "PowerUp.h"
#include "Obstacle.h"
#include "PlayerClass.h"
#include "PlayerClassDefinition.h"

AEndlessRunnerGameMode::AEndlessRunnerGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	RunnerGameState = EGameState::Menu;
	Score = 0;
	DistanceTraveled = 0.0f;
	Lives = StartingLives;
	PlayerSpawnLocation = FVector(100.0f, 0.0f, 200.0f);  // Spawn 100 units forward to avoid immediate collisions
	PlayerSpawnRotation = FRotator::ZeroRotator;
	
	// Initialize seed to 0 (will generate random on first game start if not set)
	TrackSeed = 0;
	SeededRandomStream.Initialize(TrackSeed);
	
	// Set default HUD class
	HUDClass = AEndlessRunnerHUD::StaticClass();
	
	// Set default pawn class - this will be overridden by blueprint
	// but we set it here as a fallback
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/EndlessRunner/Characters/BP_RabbitCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AEndlessRunnerGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Find existing track generator in level, or create one
	TrackGenerator = nullptr;
	for (TActorIterator<ATrackGenerator> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
	{
		TrackGenerator = *ActorIterator;
		break;
	}

	if (!TrackGenerator)
	{
		TrackGenerator = GetWorld()->SpawnActor<ATrackGenerator>(ATrackGenerator::StaticClass());
	}

	// Create gameplay manager
	GameplayManager = NewObject<UGameplayManager>(this);

	// Create currency manager
	CurrencyManager = NewObject<UCurrencyManager>(this);

	// Initialize managers
	if (GameplayManager)
	{
		GameplayManager->Initialize(this);
	}

	if (CurrencyManager)
	{
		CurrencyManager->Initialize();
	}

	// Set up Enhanced Input Mapping Context
	SetupEnhancedInput();

	// Don't auto-start - wait for player to click Play from main menu
}

void AEndlessRunnerGameMode::RestartPlayer(AController* NewPlayer)
{
	// Don't spawn pawn if we're in menu state - wait for StartGame() to be called
	if (RunnerGameState == EGameState::Menu)
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: RestartPlayer called but game state is Menu - skipping pawn spawn"));
		return;
	}
	
	// If we're playing, let StartGame() handle spawning (it has more control)
	// Only call Super if we're in a state where we want automatic spawning
	if (RunnerGameState == EGameState::Playing)
	{
		// StartGame() handles player spawning manually, so we don't need to call Super here
		// This prevents double-spawning
		UE_LOG(LogTemp, Log, TEXT("GameMode: RestartPlayer called during Playing state - StartGame() handles spawning"));
		return;
	}
	
	// For other states (Paused, GameOver), let the base class handle it
	Super::RestartPlayer(NewPlayer);
}

void AEndlessRunnerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RunnerGameState == EGameState::Playing)
	{
		// Update game time
		GameTime += DeltaTime;

		// Update distance first (cache player to avoid casting every frame)
		if (!CachedPlayer || !IsValid(CachedPlayer))
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				CachedPlayer = Cast<ARabbitCharacter>(PC->GetPawn());
			}
		}
		if (CachedPlayer)
		{
			// Track distance from spawn position (not absolute position)
			DistanceTraveled = CachedPlayer->GetActorLocation().X - PlayerSpawnLocation.X;
		}

		// Update score (throttled - every 0.1 seconds to avoid too frequent updates)
		static float ScoreUpdateTimer = 0.0f;
		ScoreUpdateTimer += DeltaTime;
		if (ScoreUpdateTimer >= 0.1f)
		{
			ScoreUpdateTimer = 0.0f;
			UpdateScore(DeltaTime);
		}

		// Update gameplay manager
		if (GameplayManager)
		{
			GameplayManager->Update(DeltaTime, DistanceTraveled);
		}

		// Update magnet effect if active
		if (bMagnetActive)
		{
			UpdateMagnetEffect(DeltaTime);
		}
	}
}

void AEndlessRunnerGameMode::SetGameState(EGameState NewState)
{
	RunnerGameState = NewState;

	// Handle state changes
	switch (RunnerGameState)
	{
	case EGameState::Playing:
		// Resume gameplay
		break;
	case EGameState::Paused:
		// Pause gameplay
		break;
	case EGameState::GameOver:
		// Handle game over
		if (CurrencyManager)
		{
			CurrencyManager->SaveCoins();
		}
		break;
	}
}

void AEndlessRunnerGameMode::StartGame()
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: StartGame() called - Current state: %d"), (int32)RunnerGameState);
	
	// Reset game state FIRST - this is critical
	SetGameState(EGameState::Playing);
	Score = 0;
	RunCurrency = 0; // Reset run currency (temporary, per-run)
	// Initialize distance tracking from spawn (starts at 0)
	DistanceTraveled = 0.0f;
	PreviousDistanceForScore = 0.0f;
	GameTime = 0.0f;
	
	// Always generate a new random seed for each new run/retry
	// (This ensures retry gets a fresh track layout)
	TrackSeed = 0;
	GenerateRandomSeed();
	
	// Reset class-specific flags
	bSpawnSpecialCollectibles = false;
	
	// Clean up any orphaned collectibles, obstacles, and power-ups that might exist
	// (in case they weren't properly registered with track pieces)
	if (UWorld* World = GetWorld())
	{
		// Destroy all collectible coins
		for (TActorIterator<ACollectibleCoin> ActorIterator(World); ActorIterator; ++ActorIterator)
		{
			if (ACollectibleCoin* Coin = *ActorIterator)
			{
				if (IsValid(Coin))
				{
					Coin->Destroy();
				}
			}
		}
		
		// Destroy all multi-collectibles
		for (TActorIterator<AMultiCollectible> ActorIterator(World); ActorIterator; ++ActorIterator)
		{
			if (AMultiCollectible* Multi = *ActorIterator)
			{
				if (IsValid(Multi))
				{
					Multi->Destroy();
				}
			}
		}
		
		// Destroy all power-ups
		for (TActorIterator<APowerUp> ActorIterator(World); ActorIterator; ++ActorIterator)
		{
			if (APowerUp* PowerUp = *ActorIterator)
			{
				if (IsValid(PowerUp))
				{
					PowerUp->Destroy();
				}
			}
		}
		
		// Destroy all obstacles
		for (TActorIterator<AObstacle> ActorIterator(World); ActorIterator; ++ActorIterator)
		{
			if (AObstacle* Obstacle = *ActorIterator)
			{
				if (IsValid(Obstacle))
				{
					Obstacle->Destroy();
				}
			}
		}
		
		UE_LOG(LogTemp, Log, TEXT("GameMode: Cleaned up all collectibles, obstacles, and power-ups"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("GameMode: StartGame() - State set to Playing, Score=%d, Lives=%d, SelectedClass=%d"), 
		Score, Lives, (int32)SelectedClass);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: No player controller found!"));
		return;
	}

	// Get or spawn player
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(PlayerController->GetPawn());
	if (!Player)
	{
		// Spawn player if not already spawned
		if (DefaultPawnClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Player = World->SpawnActor<ARabbitCharacter>(DefaultPawnClass, PlayerSpawnLocation, PlayerSpawnRotation, SpawnParams);
			if (Player)
			{
				PlayerController->Possess(Player);
				UE_LOG(LogTemp, Log, TEXT("GameMode: Spawned player at (%.2f, %.2f, %.2f)"), 
					PlayerSpawnLocation.X, PlayerSpawnLocation.Y, PlayerSpawnLocation.Z);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameMode: Failed to spawn player!"));
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: DefaultPawnClass is null!"));
			return;
		}
	}
	else
	{
		// Reset ragdoll state first (in case player died with ragdoll)
		// This must be done BEFORE setting location/rotation to avoid conflicts
		Player->ResetRagdollState();
		
		// Use the original spawn location/rotation (not the cached ones which might be wrong)
		// Match the exact spawn behavior from BeginPlay - just set to Z=200 and let movement component handle landing
		FVector ResetLocation = FVector(100.0f, 0.0f, 200.0f);  // Original spawn location - same as BeginPlay
		
		// Reset the mesh component's relative transform FIRST (before setting location/rotation)
		// Get the default relative transform from the blueprint/CDO to preserve the mesh's original position
		if (USkeletalMeshComponent* MeshComp = Player->GetMesh())
		{
			// Get the default relative transform from the class default object (blueprint defaults)
			ARabbitCharacter* CDO = Player->GetClass()->GetDefaultObject<ARabbitCharacter>();
			if (CDO && CDO->GetMesh())
			{
				// Use the blueprint's default relative transform
				FVector DefaultRelativeLocation = CDO->GetMesh()->GetRelativeLocation();
				FRotator DefaultRelativeRotation = CDO->GetMesh()->GetRelativeRotation();
				FVector DefaultRelativeScale = CDO->GetMesh()->GetRelativeScale3D();
				
				MeshComp->SetRelativeLocationAndRotation(DefaultRelativeLocation, DefaultRelativeRotation);
				MeshComp->SetRelativeScale3D(DefaultRelativeScale);
				
				UE_LOG(LogTemp, Warning, TEXT("GameMode: Reset mesh to blueprint default relative transform - Location (%.2f, %.2f, %.2f), Rotation (%.2f, %.2f, %.2f)"), 
					DefaultRelativeLocation.X, DefaultRelativeLocation.Y, DefaultRelativeLocation.Z,
					DefaultRelativeRotation.Pitch, DefaultRelativeRotation.Yaw, DefaultRelativeRotation.Roll);
			}
			else
			{
				// Fallback to zero if CDO not available
				MeshComp->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
				MeshComp->SetRelativeScale3D(FVector::OneVector);
			}
			// Force update to ensure transform is applied
			MeshComp->UpdateComponentToWorld();
		}
		
		// Set actor rotation - character should face forward (along +X)
		// Since we're now preserving the blueprint's default mesh rotation, we should use zero rotation
		FRotator ResetRotation = FRotator::ZeroRotator;  // Facing forward (blueprint mesh rotation is preserved)
		Player->SetActorRotation(ResetRotation);
		
		// Reset controller rotation to match
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->SetControlRotation(ResetRotation);
		}
		
		// Reset player to spawn location - use TeleportPhysics to ensure physics state is properly reset
		// Set location AFTER rotation to ensure proper transform
		Player->SetActorLocation(ResetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		
		// Reset lane position to center (always start in center lane)
		Player->ResetLanePosition();
		
		// Force update root component transform
		if (USceneComponent* RootComp = Player->GetRootComponent())
		{
			RootComp->UpdateComponentToWorld();
		}
		
		// Update cached spawn location/rotation
		PlayerSpawnLocation = ResetLocation;
		PlayerSpawnRotation = ResetRotation;
		
		// Initialize distance tracking (starts at 0, relative to spawn)
		DistanceTraveled = 0.0f;
		PreviousDistanceForScore = 0.0f;
		
		// Ensure player movement is enabled - let the movement component handle landing naturally
		// This matches the behavior of first spawn where BeginPlay sets Z=200 and movement handles the rest
		if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
		{
			// Set movement mode to walking - this will let the character fall and land naturally
			MovementComp->SetMovementMode(MOVE_Walking);
			
			// Set initial velocity to forward direction
			FVector ForwardDir = FVector(1.0f, 0.0f, 0.0f);
			MovementComp->Velocity = ForwardDir * Player->GetForwardSpeed();
			MovementComp->UpdateComponentVelocity();
			
			// Ensure the character is actually ticking
			Player->SetActorTickEnabled(true);
			Player->PrimaryActorTick.bCanEverTick = true;
			
			// Set forward speed on movement component
			if (URabbitMovementComponent* RabbitMovement = Cast<URabbitMovementComponent>(MovementComp))
			{
				RabbitMovement->SetForwardSpeed(Player->GetForwardSpeed());
			}
			
			// Also set MaxWalkSpeed directly
			MovementComp->MaxWalkSpeed = Player->GetForwardSpeed();
			
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Player movement setup - MaxWalkSpeed=%.2f, MovementMode=%d, TickEnabled=%d, InitialVelocity=(%.2f, %.2f, %.2f)"),
				MovementComp->MaxWalkSpeed, (int32)MovementComp->MovementMode, Player->IsActorTickEnabled(),
				MovementComp->Velocity.X, MovementComp->Velocity.Y, MovementComp->Velocity.Z);
		}
		
		UE_LOG(LogTemp, Log, TEXT("GameMode: Reset player to spawn location (%.2f, %.2f, %.2f), rotation (%.2f, %.2f, %.2f)"), 
			ResetLocation.X, ResetLocation.Y, ResetLocation.Z,
			ResetRotation.Pitch, ResetRotation.Yaw, ResetRotation.Roll);
	}

	// Update cached player reference
	CachedPlayer = Player;

	// Cache spawn location for respawn
	if (Player)
	{
		PlayerSpawnLocation = Player->GetActorLocation();
		PlayerSpawnRotation = Player->GetActorRotation();
		
		// Reset all GAS effects, multipliers, and base attributes (must be after player is spawned/reset)
		Player->ResetGASEffects();
		
		// Reset jump count
		if (URabbitJumpComponent* JumpComp = Player->GetJumpComponent())
		{
			JumpComp->ResetJumpCount();
		}
		
		// Initialize lives in GAS
		if (Player->GetAttributeSet())
		{
			Player->GetAttributeSet()->SetBaseLives(static_cast<float>(StartingLives));
			Lives = StartingLives; // Keep legacy for now
		}
		else
		{
			Lives = StartingLives;
		}
		
		// Reset forward speed to base value from attribute set (before class perks are applied)
		if (Player->GetAttributeSet())
		{
			float BaseSpeed = Player->GetAttributeSet()->GetBaseSpeed();
			Player->SetForwardSpeed(BaseSpeed);
			
			// Update movement components with the reset speed
			if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed = BaseSpeed;
				
				if (URabbitMovementComponent* RabbitMovement = Cast<URabbitMovementComponent>(MovementComp))
				{
					RabbitMovement->SetForwardSpeed(BaseSpeed);
				}
				
				// Update velocity to match new speed
				FVector ForwardDir = FVector(1.0f, 0.0f, 0.0f);
				MovementComp->Velocity = ForwardDir * BaseSpeed;
				MovementComp->UpdateComponentVelocity();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: Player is null after spawn/reset!"));
		Lives = StartingLives;
		return;
	}

	// Ensure player is possessed
	if (Player && PlayerController)
	{
		if (PlayerController->GetPawn() != Player)
		{
			PlayerController->Possess(Player);
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Possessed player character"));
		}
	}
	
	// Unpause the game FIRST - this is critical
	PlayerController->SetPause(false);
	
	// Ensure world is not paused
	if (UWorld* WorldPtr = GetWorld())
	{
		WorldPtr->GetWorldSettings()->SetPauserPlayerState(nullptr);
		bool bWorldPaused = WorldPtr->IsPaused();
		UE_LOG(LogTemp, Warning, TEXT("GameMode: After SetPause(false) - World paused=%d"), bWorldPaused ? 1 : 0);
	}
	
	PlayerController->bShowMouseCursor = false;
	PlayerController->SetInputMode(FInputModeGameOnly());
	
	// Re-enable input if it was disabled during ragdoll death
	if (DeathPlayerController == PlayerController)
	{
		PlayerController->EnableInput(PlayerController);
		DeathPlayerController = nullptr;
	}
	
	// Focus the viewport so input works immediately without clicking
	if (GEngine && GEngine->GameViewport)
	{
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Focused game viewport"));
	}
	
	// CRITICAL: Re-enable tick AFTER unpausing - this ensures tick starts
	if (Player)
	{
		// First disable, then re-enable to force a refresh
		Player->SetActorTickEnabled(false);
		
		// Force tick to be enabled with all settings
		Player->PrimaryActorTick.bCanEverTick = true;
		Player->PrimaryActorTick.TickInterval = 0.0f; // No tick interval
		Player->PrimaryActorTick.TickGroup = TG_PrePhysics;
		Player->PrimaryActorTick.bStartWithTickEnabled = true;
		
		// Now enable it
		Player->SetActorTickEnabled(true);
		
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Re-enabled player tick - TickEnabled=%d, CanEverTick=%d, StartWithTickEnabled=%d"), 
			Player->IsActorTickEnabled(), 
			Player->PrimaryActorTick.bCanEverTick,
			Player->PrimaryActorTick.bStartWithTickEnabled);
		
		// Apply class perks AFTER player is fully set up
		ApplyClassPerks(Player);
		
		// Enable invincibility for 10 seconds at game start
		Player->SetInvincible(true, 10.0f);
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Enabled 10 second invincibility period"));
		
		// Ensure input component is set up (this is called automatically on possession, but ensure it's done)
		if (UInputComponent* InputComp = Player->InputComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Player has InputComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Player does NOT have InputComponent - SetupPlayerInputComponent may not have been called"));
		}
		
		// Log tick state
		UE_LOG(LogTemp, Warning, TEXT("GameMode: After unpause - TickEnabled=%d, CanEverTick=%d, IsPossessed=%d"), 
			Player->IsActorTickEnabled(), 
			Player->PrimaryActorTick.bCanEverTick,
			PlayerController && PlayerController->GetPawn() == Player ? 1 : 0);
		
		// Force movement component to be active and ticking
		if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
		{
			MovementComp->SetActive(true);
			MovementComp->SetComponentTickEnabled(true);
			MovementComp->PrimaryComponentTick.bCanEverTick = true;
			MovementComp->PrimaryComponentTick.TickGroup = TG_PrePhysics;
			
			// Set velocity immediately
			FVector ForwardDir = FVector(1.0f, 0.0f, 0.0f);
			MovementComp->Velocity = ForwardDir * Player->GetForwardSpeed();
			MovementComp->SetMovementMode(MOVE_Walking);
			MovementComp->UpdateComponentVelocity();
			
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Movement component setup - Velocity=(%.2f, %.2f, %.2f), ComponentTickEnabled=%d"),
				MovementComp->Velocity.X, MovementComp->Velocity.Y, MovementComp->Velocity.Z,
				MovementComp->IsComponentTickEnabled());
		}
		
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Player tick setup - TickEnabled=%d, CanEverTick=%d"), 
			Player->IsActorTickEnabled(), Player->PrimaryActorTick.bCanEverTick);
	}
	
	// Show in-game HUD (this should NOT pause the game)
	// IMPORTANT: ShowInGameHUD() will remove all widgets including game over screen
	if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
	{
		// Ensure game state is Playing before showing HUD
		if (RunnerGameState != EGameState::Playing)
		{
			UE_LOG(LogTemp, Error, TEXT("GameMode: StartGame() - Game state is NOT Playing (%d), forcing to Playing"), (int32)RunnerGameState);
			SetGameState(EGameState::Playing);
		}
		
		HUD->ShowInGameHUD();
		
		// Ensure game is still unpaused after showing HUD
		PlayerController->SetPause(false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
		
		// Re-enable input if it was disabled during ragdoll death
		if (DeathPlayerController == PlayerController)
		{
			PlayerController->EnableInput(PlayerController);
			DeathPlayerController = nullptr;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("GameMode: StartGame() - In-game HUD shown, game unpaused"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: StartGame() - HUD is null!"));
	}

	// Reset and initialize track generator
	if (TrackGenerator)
	{
		// Reset track generator (clears all pieces)
		TrackGenerator->Reset();
		
		// Initialize with player
		if (Player)
		{
			TrackGenerator->Initialize(Player);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: Player is null, cannot initialize track"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: TrackGenerator is null, cannot initialize track"));
	}

	// Reset gameplay manager
	if (GameplayManager)
	{
		GameplayManager->Reset();
	}

	// Note: RunCurrency is reset in StartGame(), persistent Coins are managed by CurrencyManager
}

void AEndlessRunnerGameMode::PauseGame()
{
	if (RunnerGameState == EGameState::Playing)
	{
		SetGameState(EGameState::Paused);
		
		// Pause the game
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->SetPause(true);
			}
		}
	}
}

void AEndlessRunnerGameMode::ResumeGame()
{
	if (RunnerGameState == EGameState::Paused)
	{
		SetGameState(EGameState::Playing);
		
		// Unpause the game
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->SetPause(false);
			}
		}
	}
}

void AEndlessRunnerGameMode::EndGame()
{
	SetGameState(EGameState::GameOver);
	
	// Pause the game
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
			
			// Show game over screen
			if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
			{
				HUD->ShowGameOverScreen();
			}
		}
	}
}

void AEndlessRunnerGameMode::AddScore(int32 Points)
{
	// Apply score multiplier from power-ups (GAS only)
	ARabbitCharacter* Player = GetCachedPlayer();
	float EffectiveScoreMultiplier = 1.0f;
	if (Player && Player->GetAttributeSet())
	{
		EffectiveScoreMultiplier = Player->GetScoreMultiplier();
	}
	
	if (EffectiveScoreMultiplier != 1.0f)
	{
		Points = FMath::FloorToInt(Points * EffectiveScoreMultiplier);
	}
	
	// Apply last legs multiplier if on last legs
	if (IsOnLastLegs())
	{
		// Check if Joker class - scale multiplier with speed
		float EffectiveMultiplier = LastLegsScoreMultiplier;
		FPlayerClassData ClassData = GetClassData(SelectedClass);
		if (ClassData.bScalesLastLegsWithSpeed)
		{
			// Get speed multiplier from GAS
			float CurrentSpeedMultiplier = 1.0f;
			if (Player && Player->GetAttributeSet())
			{
				CurrentSpeedMultiplier = 1.0f + Player->GetAttributeSet()->GetSpeedMultiplier();
			}
			
			EffectiveMultiplier = LastLegsScoreMultiplier * CurrentSpeedMultiplier;
			UE_LOG(LogTemp, Log, TEXT("GameMode: Joker class - Last legs multiplier scaled by speed: %.2f * %.2f = %.2f"), 
				LastLegsScoreMultiplier, CurrentSpeedMultiplier, EffectiveMultiplier);
		}
		
		Points = FMath::FloorToInt(Points * EffectiveMultiplier);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Last legs mode - score multiplied by %.2f"), EffectiveMultiplier);
	}
	
	Score += Points;
}

void AEndlessRunnerGameMode::UpdateScore(float DeltaTime)
{
	// Use the DistanceTraveled that's already being tracked in Tick()
	// Calculate distance delta since last update
	float DistanceDelta = DistanceTraveled - PreviousDistanceForScore;

	// Only update if there's actual forward movement
	if (DistanceDelta > 0.0f)
	{
		// Update previous distance only when we process movement
		PreviousDistanceForScore = DistanceTraveled;
		
		// Add score based on distance (convert to meters: 1 meter = 800 units)
		float DistanceDeltaMeters = DistanceDelta / 800.0f;
		int32 PointsToAdd = FMath::FloorToInt(DistanceDeltaMeters * PointsPerMeter);
		if (PointsToAdd > 0)
		{
			AddScore(PointsToAdd);
			UE_LOG(LogTemp, VeryVerbose, TEXT("GameMode: Added %d points (%.2f meters, delta=%.2f units, total distance=%.2f)"), 
				PointsToAdd, DistanceDeltaMeters, DistanceDelta, DistanceTraveled);
		}
	}
}

void AEndlessRunnerGameMode::SetupEnhancedInput()
{
	// Get the player controller
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	// Get the local player
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	// Get the Enhanced Input subsystem
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	// Load the Input Mapping Context at runtime
	UInputMappingContext* IMC_Keyboard = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/EndlessRunner/Input/IMC_Keyboard"));
	if (IMC_Keyboard)
	{
		// Add the mapping context with priority 0
		InputSubsystem->AddMappingContext(IMC_Keyboard, 0);
		UE_LOG(LogTemp, Log, TEXT("EndlessRunnerGameMode: Added IMC_Keyboard to Enhanced Input"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EndlessRunnerGameMode: Could not load IMC_Keyboard at /Game/EndlessRunner/Input/IMC_Keyboard"));
	}
}

void AEndlessRunnerGameMode::OnPlayerHitObstacle(int32 LivesLost, bool bInstantDeath)
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: OnPlayerHitObstacle called with LivesLost=%d, bInstantDeath=%d, CurrentState=%d"), 
		LivesLost, bInstantDeath ? 1 : 0, (int32)RunnerGameState);
	
	if (RunnerGameState != EGameState::Playing)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameMode: OnPlayerHitObstacle ignored - game not in Playing state"));
		return;
	}

	// Get current lives from GAS
	ARabbitCharacter* Player = GetCachedPlayer();
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: OnPlayerHitObstacle - Player is null!"));
		return;
	}

	// If instant death is enabled, kill player immediately regardless of lives
	if (bInstantDeath)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: Instant death obstacle - killing player immediately (bypasses last stand)"));
		
		// Enable ragdoll death
		if (Player)
		{
			// Calculate launch velocity (forward and up)
			FVector ForwardDir = Player->GetActorForwardVector();
			FVector LaunchVelocity = ForwardDir * 2000.0f + FVector(0.0f, 0.0f, 2500.0f);
			
			// Enable ragdoll and launch
			Player->EnableRagdollDeath(LaunchVelocity);
			
			// Disable input
			if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
			{
				PlayerController->DisableInput(PlayerController);
			}
		}
		
		// Set lives to 0
		if (Player && Player->GetAttributeSet())
		{
			Player->GetAttributeSet()->SetBaseLives(0.0f);
		}
		Lives = 0;
		
		// Set game state to GameOver
		SetGameState(EGameState::GameOver);
		
		// Show game over screen
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeUIOnly());
				
				if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
				{
					HUD->ShowGameOverScreen();
				}
			}
		}
		return;
	}

	// Apply damage reduction based on class
	FPlayerClassData ClassData = GetClassData(SelectedClass);
	float ActualLivesLost = static_cast<float>(LivesLost);
	if (ClassData.DamageReduction < 1.0f)
	{
		ActualLivesLost = ActualLivesLost * ClassData.DamageReduction;
		UE_LOG(LogTemp, Log, TEXT("GameMode: Damage reduction applied - Original: %d, Reduced: %.2f"), LivesLost, ActualLivesLost);
	}
	int32 RoundedLivesLost = FMath::Max(1, FMath::RoundToInt(ActualLivesLost)); // At least 1 life lost

	int32 CurrentLives = GetLives();
	
	// Check if already on last legs (0 lives)
	bool bWasOnLastLegs = (CurrentLives <= 0);

	// Reduce lives in both GAS and legacy
	int32 NewLives = FMath::Max(0, CurrentLives - RoundedLivesLost);
	
	// Update GAS first
	if (Player && Player->GetAttributeSet())
	{
		Player->GetAttributeSet()->SetBaseLives(static_cast<float>(NewLives));
	}
	
	// Update legacy variable
	Lives = NewLives;
	
	UE_LOG(LogTemp, Warning, TEXT("GameMode: Player hit obstacle - Lost %d lives (after reduction), Remaining: %d"), RoundedLivesLost, NewLives);

	if (bWasOnLastLegs)
	{
		// Player was already on last legs - this hit kills them
		UE_LOG(LogTemp, Error, TEXT("GameMode: Player died on last legs - enabling ragdoll death"));
		
		// Enable ragdoll death (Player already retrieved above)
		if (Player)
		{
			// Calculate launch velocity (forward and up)
			FVector ForwardDir = Player->GetActorForwardVector();
			FVector LaunchVelocity = ForwardDir * 2000.0f + FVector(0.0f, 0.0f, 2500.0f); // Forward + more upward force
			
			// Enable ragdoll and launch
			Player->EnableRagdollDeath(LaunchVelocity);
			
			// Disable input
			if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
			{
				PlayerController->DisableInput(PlayerController);
			}
		}
		
		// Set game state to GameOver but don't pause
		SetGameState(EGameState::GameOver);
		
		// Show game over screen without pausing
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeUIOnly());
				
				// Show game over screen
				if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
				{
					HUD->ShowGameOverScreen();
				}
			}
		}
	}
	else if (IsOnLastLegs())
	{
		// Just entered last legs mode (0 lives remaining)
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Player on last legs - score multiplier active"));
		// Don't respawn - player continues with 0 lives
	}
	else
	{
		// Still has lives - player continues playing (no respawn)
		// Just lose a life and keep going
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Player hit obstacle, lost life but continues playing"));
	}
}

void AEndlessRunnerGameMode::RespawnPlayer()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	// Find ground position at spawn location
	// First try to find nearest track piece, then use line trace as fallback
	FVector SpawnLocation = PlayerSpawnLocation;
	float GroundZ = 0.0f; // Default ground level (track pieces are at Z=0)
	bool bFoundGround = false;
	
	if (UWorld* World = GetWorld())
	{
		// Try to find nearest track piece to get ground level
		if (TrackGenerator)
		{
			// Get all active track pieces
			TArray<ATrackPiece*> ActivePieces = TrackGenerator->GetActiveTrackPieces();
			float MinDistance = FLT_MAX;
			ATrackPiece* NearestPiece = nullptr;
			
			for (ATrackPiece* Piece : ActivePieces)
			{
				if (Piece && IsValid(Piece))
				{
					FVector PieceLocation = Piece->GetActorLocation();
					float Distance = FMath::Abs(PieceLocation.X - PlayerSpawnLocation.X);
					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						NearestPiece = Piece;
					}
				}
			}
			
			if (NearestPiece)
			{
				// Use the track piece's ground level (Z=0 relative to piece)
				GroundZ = NearestPiece->GetActorLocation().Z;
				bFoundGround = true;
				UE_LOG(LogTemp, Warning, TEXT("GameMode: Found nearest track piece at Z=%.2f, using as ground level"), GroundZ);
			}
		}
		
		// Fallback: Try line trace with multiple collision channels
		if (!bFoundGround)
		{
			FHitResult HitResult;
			FVector TraceStart = FVector(PlayerSpawnLocation.X, PlayerSpawnLocation.Y, PlayerSpawnLocation.Z + 1000.0f);
			FVector TraceEnd = FVector(PlayerSpawnLocation.X, PlayerSpawnLocation.Y, PlayerSpawnLocation.Z - 1000.0f);
			
			FCollisionQueryParams QueryParams;
			QueryParams.bTraceComplex = false;
			QueryParams.bReturnPhysicalMaterial = false;
			
			// Try multiple collision channels
			FCollisionObjectQueryParams ObjectParams;
			ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
			ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
			ObjectParams.AddObjectTypesToQuery(ECC_Pawn); // Track pieces might be pawns
			
			if (World->LineTraceSingleByObjectType(HitResult, TraceStart, TraceEnd, ObjectParams, QueryParams))
			{
				GroundZ = HitResult.ImpactPoint.Z;
				bFoundGround = true;
				UE_LOG(LogTemp, Warning, TEXT("GameMode: Found ground via line trace at Z=%.2f"), GroundZ);
			}
		}
		
		// Place player 200 units above ground
		SpawnLocation.Z = GroundZ + 200.0f;
		if (!bFoundGround)
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: No ground found, using default Z=%.2f"), SpawnLocation.Z);
		}
	}

	// Get or spawn player
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(PlayerController->GetPawn());
	if (!Player)
	{
		// Spawn new player
		if (DefaultPawnClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Player = GetWorld()->SpawnActor<ARabbitCharacter>(DefaultPawnClass, SpawnLocation, PlayerSpawnRotation, SpawnParams);
			if (Player)
			{
				PlayerController->Possess(Player);
				UE_LOG(LogTemp, Log, TEXT("GameMode: Respawned player at (%.2f, %.2f, %.2f)"), 
					SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
			}
		}
	}
	else
	{
		// Teleport existing player to spawn
		Player->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Player->SetActorRotation(PlayerSpawnRotation);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Respawned player at (%.2f, %.2f, %.2f)"), 
			SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);
	}

	// Update cached player reference
	CachedPlayer = Player;

	// Reset player movement state
	if (Player)
	{
		// Reset all GAS effects and multipliers on respawn
		Player->ResetGASEffects();
		
		if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
		{
			MovementComp->Velocity = FVector::ZeroVector;
			MovementComp->SetMovementMode(MOVE_Walking);
			MovementComp->UpdateComponentVelocity();
		}
		
		// Re-enable invincibility for a short period after respawn
		Player->SetInvincible(true, 2.0f);
	}

	// Update track generator player reference if needed (don't reinitialize - that resets spawn position!)
	if (TrackGenerator && Player)
	{
		// Only update player reference, don't reset the track
		// The track generator should continue spawning pieces normally
		TrackGenerator->UpdatePlayerReference(Player);
	}
}

float AEndlessRunnerGameMode::GetPlayerSpeed() const
{
	// Use cached player if available (const cast needed for const function)
	AEndlessRunnerGameMode* NonConstThis = const_cast<AEndlessRunnerGameMode*>(this);
	if (!NonConstThis->CachedPlayer || !IsValid(NonConstThis->CachedPlayer))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			NonConstThis->CachedPlayer = Cast<ARabbitCharacter>(PC->GetPawn());
		}
	}
	
	ARabbitCharacter* Player = NonConstThis->CachedPlayer;
	if (Player && Player->GetCharacterMovement())
	{
		return Player->GetCharacterMovement()->Velocity.Size();
	}
	return 0.0f;
}

int32 AEndlessRunnerGameMode::GetMaxJumpCount() const
{
	// Use cached player if available (const cast needed for const function)
	AEndlessRunnerGameMode* NonConstThis = const_cast<AEndlessRunnerGameMode*>(this);
	if (!NonConstThis->CachedPlayer || !IsValid(NonConstThis->CachedPlayer))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			NonConstThis->CachedPlayer = Cast<ARabbitCharacter>(PC->GetPawn());
		}
	}
	
	ARabbitCharacter* Player = NonConstThis->CachedPlayer;
	if (Player)
	{
		return Player->GetCurrentMaxJumpCount();
	}
	
	// Fallback to default if player not available
	return 1;
}

int32 AEndlessRunnerGameMode::GetCurrentJumpCount() const
{
	// Use cached player if available (const cast needed for const function)
	AEndlessRunnerGameMode* NonConstThis = const_cast<AEndlessRunnerGameMode*>(this);
	if (!NonConstThis->CachedPlayer || !IsValid(NonConstThis->CachedPlayer))
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			NonConstThis->CachedPlayer = Cast<ARabbitCharacter>(PC->GetPawn());
		}
	}
	
	ARabbitCharacter* Player = NonConstThis->CachedPlayer;
	if (Player && Player->GetJumpComponent())
	{
		return Player->GetJumpComponent()->GetCurrentJumpCount();
	}
	
	// Fallback to 0 if player not available
	return 0;
}

FString AEndlessRunnerGameMode::GetActivePowerUpStatus() const
{
	TArray<FString> ActiveEffects;
	
	// Check multipliers from GAS
	ARabbitCharacter* Player = GetCachedPlayer();
	if (Player && Player->GetAttributeSet())
	{
		float SpeedMult = 1.0f + Player->GetAttributeSet()->GetSpeedMultiplier();
		if (SpeedMult != 1.0f)
		{
			ActiveEffects.Add(FString::Printf(TEXT("Speed x%.1f"), SpeedMult));
		}
		
		float CoinMult = Player->GetCoinMultiplier();
		if (CoinMult != 1.0f)
		{
			ActiveEffects.Add(FString::Printf(TEXT("Coins x%.1f"), CoinMult));
		}
		
		float ScoreMult = Player->GetScoreMultiplier();
		if (ScoreMult != 1.0f)
		{
			ActiveEffects.Add(FString::Printf(TEXT("Score x%.1f"), ScoreMult));
		}
	}
	
	if (CachedPlayer && CachedPlayer->IsInvincible())
	{
		ActiveEffects.Add(TEXT("Invincible"));
	}
	
	if (bMagnetActive)
	{
		ActiveEffects.Add(TEXT("Magnet"));
	}
	
	if (bAutopilotActive)
	{
		ActiveEffects.Add(TEXT("Autopilot"));
	}
	
	if (ActiveEffects.Num() > 0)
	{
		return FString::Join(ActiveEffects, TEXT(" | "));
	}
	return TEXT("");
}

void AEndlessRunnerGameMode::AddRunCurrency(int32 Amount)
{
	// Apply coin multiplier from power-ups (GAS only)
	ARabbitCharacter* Player = GetCachedPlayer();
	if (Player && Player->GetAttributeSet())
	{
		float CoinMult = Player->GetCoinMultiplier();
		if (CoinMult != 1.0f)
		{
			Amount = FMath::FloorToInt(Amount * CoinMult);
		}
	}
	
	RunCurrency += Amount;
	UE_LOG(LogTemp, Log, TEXT("GameMode: Added %d to RunCurrency (Total: %d)"), Amount, RunCurrency);
}

void AEndlessRunnerGameMode::ClearPowerUpInvincibility()
{
	// Note: Player's own invincibility timer handles this, but we clear our timer
	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
}

void AEndlessRunnerGameMode::ClearGameModeInvincibility()
{
	GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
	ARabbitCharacter* Player = GetCachedPlayer();
	if (Player) Player->SetInvincible(false);
	UE_LOG(LogTemp, Log, TEXT("GameMode: GameMode invincibility tracking cleared."));
}

ARabbitCharacter* AEndlessRunnerGameMode::GetCachedPlayer() const
{
	if (CachedPlayer && IsValid(CachedPlayer))
	{
		return CachedPlayer;
	}
	// Fallback to finding player
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			return Cast<ARabbitCharacter>(PC->GetPawn());
		}
	}
	return nullptr;
}

void AEndlessRunnerGameMode::UpdateMagnetEffect(float DeltaTime)
{
	ARabbitCharacter* Player = GetCachedPlayer();
	if (!Player)
	{
		return;
	}

	FVector PlayerLocation = Player->GetActorLocation();
	
	// Calculate effective magnet speed based on player's current speed
	// Coins move at base magnet speed plus player's forward speed to keep up
	float PlayerSpeed = Player->GetForwardSpeed();
	float EffectiveMagnetSpeed = MagnetSpeed + PlayerSpeed;
	
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Find all magnetable collectibles within range
	for (TActorIterator<ACollectibleCoin> CoinIterator(World); CoinIterator; ++CoinIterator)
	{
		ACollectibleCoin* Coin = *CoinIterator;
		if (!Coin || !IsValid(Coin) || Coin->IsCollected())
		{
			continue;
		}

		// Check if coin is magnetable
		if (!Coin->IsMagnetable())
		{
			continue;
		}

		FVector CoinLocation = Coin->GetActorLocation();
		// Use full 3D distance to attract coins above/below and in adjacent lanes
		float Distance = FVector::Dist(PlayerLocation, CoinLocation);
		
		if (Distance <= MagnetRange)
		{
			// Attract coin toward player using speed-relative magnet speed
			FVector Direction = (PlayerLocation - CoinLocation).GetSafeNormal();
			FVector NewLocation = CoinLocation + Direction * EffectiveMagnetSpeed * DeltaTime;
			Coin->SetActorLocation(NewLocation, false, nullptr, ETeleportType::None);
			
			// Check if coin is now close enough to collect (within collection radius)
			// Use a small threshold so coins are visible longer before collection
			float CollectionDistance = 20.0f; // Close to character for longer visibility
			float NewDistance = FVector::Dist(PlayerLocation, NewLocation);
			if (NewDistance <= CollectionDistance && !Coin->IsCollected())
			{
				// Manually trigger collection since fast movement might miss overlap events
				Coin->Collect();
			}
			else
			{
				// Update overlaps to ensure physics system detects the movement
				if (USphereComponent* CollisionSphere = Coin->GetCollisionSphere())
				{
					CollisionSphere->UpdateOverlaps();
				}
			}
		}
	}

	// Also handle MultiCollectible items - attract individual items, not the whole actor
	for (TActorIterator<AMultiCollectible> MultiIterator(World); MultiIterator; ++MultiIterator)
	{
		AMultiCollectible* MultiCollectible = *MultiIterator;
		if (!MultiCollectible || !IsValid(MultiCollectible) || !MultiCollectible->IsMagnetable())
		{
			continue;
		}

		// Get individual items and attract each one separately
		const TArray<FCollectibleItem>& Items = MultiCollectible->GetCollectibleItems();
		FVector ActorLocation = MultiCollectible->GetActorLocation();
		FRotator ActorRotation = MultiCollectible->GetActorRotation();
		
		for (int32 ItemIndex = 0; ItemIndex < Items.Num(); ++ItemIndex)
		{
			const FCollectibleItem& Item = Items[ItemIndex];
			
			// Skip collected items
			if (Item.bCollected || !Item.MeshComponent)
			{
				continue;
			}

			// Get the mesh component's world location
			FVector ItemLocation = Item.MeshComponent->GetComponentLocation();
			// Use full 3D distance to attract items above/below and in adjacent lanes
			float Distance = FVector::Dist(PlayerLocation, ItemLocation);
			
			if (Distance <= MagnetRange)
			{
				// Attract this individual item toward player using speed-relative magnet speed
				FVector Direction = (PlayerLocation - ItemLocation).GetSafeNormal();
				FVector NewWorldLocation = ItemLocation + Direction * EffectiveMagnetSpeed * DeltaTime;
				
				// Convert new world location to relative location
				// Since the actor might have rotation, we need to transform it properly
				FVector OffsetFromActor = NewWorldLocation - ActorLocation;
				FVector NewRelativeLocation = ActorRotation.UnrotateVector(OffsetFromActor);
				
				// Set the mesh component's relative location
				Item.MeshComponent->SetRelativeLocation(NewRelativeLocation, false, nullptr, ETeleportType::None);
				
				// Check if item is now close enough to collect (within collection radius)
				// Use a small threshold so items are visible longer before collection
				float CollectionDistance = 20.0f; // Close to character for longer visibility
				float NewDistance = FVector::Dist(PlayerLocation, NewWorldLocation);
				if (NewDistance <= CollectionDistance && !Item.bCollected)
				{
					// Manually trigger collection since fast movement might miss overlap events
					MultiCollectible->CollectItem(ItemIndex);
				}
				else if (!Item.bCollected)
				{
					// Update overlaps to ensure physics system detects the movement
					if (Item.CollisionSphere)
					{
						Item.CollisionSphere->UpdateOverlaps();
					}
				}
			}
		}
	}
}

void AEndlessRunnerGameMode::ClearMagnet()
{
	bMagnetActive = false;
	GetWorldTimerManager().ClearTimer(MagnetTimerHandle);
	UE_LOG(LogTemp, Log, TEXT("GameMode: Magnet power-up cleared"));
}

void AEndlessRunnerGameMode::ClearAutopilot()
{
	bAutopilotActive = false;
	if (ARabbitCharacter* Player = GetCachedPlayer())
	{
		Player->SetAutopilot(false);
	}
	GetWorldTimerManager().ClearTimer(AutopilotTimerHandle);
	UE_LOG(LogTemp, Log, TEXT("GameMode: Autopilot power-up cleared"));
}

FPlayerClassData AEndlessRunnerGameMode::GetClassData(EPlayerClass Class) const
{
	if (UPlayerClassDefinition* ClassDef = GetClassDefinition(Class))
	{
		return ClassDef->ClassData;
	}
	
	// Return default/empty data if no data asset found
	return FPlayerClassData();
}

UPlayerClassDefinition* AEndlessRunnerGameMode::GetClassDefinition(EPlayerClass Class) const
{
	for (UPlayerClassDefinition* ClassDef : PlayerClassDefinitions)
	{
		if (ClassDef && ClassDef->ClassType == Class)
		{
			return ClassDef;
		}
	}
	return nullptr;
}

UPlayerClassDefinition* AEndlessRunnerGameMode::GetSelectedClassDefinition() const
{
	return GetClassDefinition(SelectedClass);
}

void AEndlessRunnerGameMode::ApplyClassPerks(ARabbitCharacter* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode: ApplyClassPerks called with null player"));
		return;
	}

	FPlayerClassData ClassData = GetClassData(SelectedClass);
	
	UE_LOG(LogTemp, Warning, TEXT("GameMode: Applying class perks for class %d"), (int32)SelectedClass);

	// Apply extra lives (Enforcer) - set in GAS
	if (ClassData.ExtraLives > 0)
	{
		float TotalLives = static_cast<float>(StartingLives + ClassData.ExtraLives);
		if (Player->GetAttributeSet())
		{
			Player->GetAttributeSet()->SetBaseLives(TotalLives);
		}
		Lives = StartingLives + ClassData.ExtraLives; // Keep legacy for now
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied %d extra lives (Total: %d)"), ClassData.ExtraLives, Lives);
	}

	// Apply max jump count (Rogue) - always set it (even if 1) to ensure it's reset properly
	Player->SetMaxJumpCount(ClassData.MaxJumpCount);
	if (URabbitJumpComponent* JumpComp = Player->GetJumpComponent())
	{
		JumpComp->ResetJumpCount(); // Ensure jump count is reset after setting max
	}
	UE_LOG(LogTemp, Log, TEXT("GameMode: Applied max jump count: %d"), ClassData.MaxJumpCount);

	// Apply capsule size (Scout)
	if (ClassData.CapsuleHalfHeight > 0.0f)
	{
		// Get current radius to preserve it
		if (UCapsuleComponent* CapsuleComp = Player->GetCapsuleComponent())
		{
			float CurrentRadius = CapsuleComp->GetUnscaledCapsuleRadius();
			Player->SetCapsuleSize(ClassData.CapsuleHalfHeight, CurrentRadius);
			UE_LOG(LogTemp, Log, TEXT("GameMode: Applied capsule half height: %.2f"), ClassData.CapsuleHalfHeight);
		}
	}

	// Apply never needs crouch (Scout)
	if (ClassData.bNeverNeedsCrouch)
	{
		Player->SetNeverNeedsCrouch(true);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied never needs crouch"));
	}

	// Apply can break obstacles (Enforcer)
	if (ClassData.bCanBreakObstacles)
	{
		Player->SetCanBreakObstacles(true);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied can break obstacles"));
	}

	// Apply spawn special collectibles (Collector)
	if (ClassData.bSpawnsSpecialCollectibles)
	{
		bSpawnSpecialCollectibles = true;
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied spawn special collectibles"));
	}

	// Apply starting magnet (for future classes)
	if (ClassData.bHasStartingMagnet && ClassData.StartingMagnetDuration > 0.0f)
	{
		// Activate magnet directly (should be migrated to GAS in the future)
		bMagnetActive = true;
		GetWorldTimerManager().SetTimer(MagnetTimerHandle, this, &AEndlessRunnerGameMode::ClearMagnet, ClassData.StartingMagnetDuration, false);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied starting magnet for %.2f seconds"), ClassData.StartingMagnetDuration);
	}

	// Apply base speed multiplier (for future classes)
	if (ClassData.BaseSpeedMultiplier != 1.0f)
	{
		float CurrentSpeed = Player->GetForwardSpeed();
		float NewSpeed = CurrentSpeed * ClassData.BaseSpeedMultiplier;
		Player->SetForwardSpeed(NewSpeed);
		UE_LOG(LogTemp, Log, TEXT("GameMode: Applied base speed multiplier: %.2f (Speed: %.2f -> %.2f)"), 
			ClassData.BaseSpeedMultiplier, CurrentSpeed, NewSpeed);
	}
}

int32 AEndlessRunnerGameMode::GetLives() const
{
	// Get from GAS first
	const ARabbitCharacter* Player = GetCachedPlayer();
	if (Player && Player->GetAttributeSet())
	{
		return FMath::RoundToInt(Player->GetAttributeSet()->GetCurrentLives());
	}
	// Legacy fallback
	return Lives;
}

bool AEndlessRunnerGameMode::IsOnLastLegs() const
{
	return GetLives() <= 0;
}

void AEndlessRunnerGameMode::SetTrackSeed(int32 NewSeed)
{
	// Sanitize seed: handle negative seeds and extremely large seeds
	if (NewSeed < 0)
	{
		// Convert negative to positive (preserve determinism)
		NewSeed = FMath::Abs(NewSeed);
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Negative seed converted to positive: %d"), NewSeed);
	}
	
	// Handle extremely large seeds (modulo to reasonable range)
	// Using a large prime number to avoid patterns
	const int32 MaxSeed = 2147483647; // INT32_MAX
	if (NewSeed > MaxSeed)
	{
		NewSeed = NewSeed % MaxSeed;
		UE_LOG(LogTemp, Warning, TEXT("GameMode: Large seed modulo'd to: %d"), NewSeed);
	}
	
	TrackSeed = NewSeed;
	SeededRandomStream.Initialize(TrackSeed);
	
	UE_LOG(LogTemp, Log, TEXT("GameMode: Track seed set to %d"), TrackSeed);
}

void AEndlessRunnerGameMode::GenerateRandomSeed()
{
	// Generate a random seed using the global random stream
	TrackSeed = FMath::RandRange(1, 2147483647); // Avoid 0 (0 might be used as "no seed")
	SeededRandomStream.Initialize(TrackSeed);
	
	UE_LOG(LogTemp, Log, TEXT("GameMode: Generated random track seed: %d"), TrackSeed);
}


