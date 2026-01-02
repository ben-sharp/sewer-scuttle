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
#include "PowerUpDefinition.h"
#include "ShopItemDefinition.h"
#include "Obstacle.h"
#include "PlayerClass.h"
#include "PlayerClassDefinition.h"
#include "WebServerInterface.h"
#include "ContentRegistry.h"
#include "DeviceIdManager.h"
#include "Misc/DateTime.h"
#include "AssetRegistry/AssetRegistryModule.h"

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
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/EndlessRunner/Characters/BP_RabbitCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AEndlessRunnerGameMode::BeginPlay()
{
	Super::BeginPlay();

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

	ContentRegistry = NewObject<UContentRegistry>(this);
	ContentRegistry->GatherContent();

	GameplayManager = NewObject<UGameplayManager>(this);
	CurrencyManager = NewObject<UCurrencyManager>(this);

	if (GameplayManager)
	{
		GameplayManager->Initialize(this);
	}

	if (CurrencyManager)
	{
		CurrencyManager->Initialize();
	}

	SetupEnhancedInput();
}

void AEndlessRunnerGameMode::RestartPlayer(AController* NewPlayer)
{
	if (RunnerGameState == EGameState::Menu)
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: RestartPlayer called but game state is Menu - skipping pawn spawn"));
		return;
	}
	
	if (RunnerGameState == EGameState::Playing)
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: RestartPlayer called during Playing state - StartGame() handles spawning"));
		return;
	}
	
	Super::RestartPlayer(NewPlayer);
}

void AEndlessRunnerGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RunnerGameState == EGameState::Playing)
	{
		GameTime += DeltaTime;

		if (!CachedPlayer || !IsValid(CachedPlayer))
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				CachedPlayer = Cast<ARabbitCharacter>(PC->GetPawn());
			}
		}
		if (CachedPlayer)
		{
			DistanceTraveled = TotalPreviousDistance + (CachedPlayer->GetActorLocation().X - PlayerSpawnLocation.X);
		}

		static float ScoreUpdateTimer = 0.0f;
		ScoreUpdateTimer += DeltaTime;
		if (ScoreUpdateTimer >= 0.1f)
		{
			ScoreUpdateTimer = 0.0f;
			UpdateScore(DeltaTime);
		}

		if (GameplayManager)
		{
			GameplayManager->Update(DeltaTime, DistanceTraveled);
		}

		if (bMagnetActive)
		{
			UpdateMagnetEffect(DeltaTime);
		}
	}
}

void AEndlessRunnerGameMode::SetGameState(EGameState NewState)
{
	RunnerGameState = NewState;

	switch (RunnerGameState)
	{
	case EGameState::Playing:
		break;
	case EGameState::Paused:
		break;
	case EGameState::GameOver:
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
	bIsEndlessMode = false;
	bTrackSequenceLoaded = false;
	
	if (!WebServerInterface)
	{
		WebServerInterface = NewObject<UWebServerInterface>(this);
		WebServerInterface->Initialize();
		
		FOnSeedReceived OnSeedReceivedDelegate;
		OnSeedReceivedDelegate.BindUFunction(this, FName("OnSeedReceived"));
		WebServerInterface->SetOnSeedReceived(OnSeedReceivedDelegate);
		
		FOnTrackSelectionReceived OnTrackSelectionReceivedDelegate;
		OnTrackSelectionReceivedDelegate.BindUFunction(this, FName("OnTrackSelectionReceived"));
		WebServerInterface->SetOnTrackSelectionReceived(OnTrackSelectionReceivedDelegate);
		
		FOnTrackSequenceReceived OnTrackSequenceReceivedDelegate;
		OnTrackSequenceReceivedDelegate.BindUFunction(this, FName("OnTrackSequenceReceived"));
		WebServerInterface->SetOnTrackSequenceReceived(OnTrackSequenceReceivedDelegate);
		
		FOnShopItemsReceived OnShopItemsReceivedDelegate;
		OnShopItemsReceivedDelegate.BindUFunction(this, FName("OnShopItemsReceived"));
		WebServerInterface->SetOnShopItemsReceived(OnShopItemsReceivedDelegate);
		
		FOnBossRewardsReceived OnBossRewardsReceivedDelegate;
		OnBossRewardsReceivedDelegate.BindUFunction(this, FName("OnBossRewardsReceived"));
		WebServerInterface->SetOnBossRewardsReceived(OnBossRewardsReceivedDelegate);
		
		FOnError OnErrorDelegate;
		OnErrorDelegate.BindUFunction(this, FName("OnSeedRequestError"));
		WebServerInterface->SetOnError(OnErrorDelegate);
	}

	WebServerInterface->RequestRunSeed(0, SelectedClass);
}

void AEndlessRunnerGameMode::StartGameWithSeed(int32 Seed, const FString& InSeedId, int32 InMaxCoins, int32 InMaxObstacles, int32 InMaxTrackPieces)
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: StartGameWithSeed() called - Seed: %d, SeedId: %s"), Seed, *InSeedId);
	
	SetGameState(EGameState::Playing);
	Score = 0;
	RunCurrency = 0;
	ObstaclesHit = 0;
	PowerupsUsed = 0;
	DistanceTraveled = 0.0f;
	PreviousDistanceForScore = 0.0f;
	GameTime = 0.0f;
	
	TrackSeed = Seed;
	SeedId = InSeedId;
	MaxCoins = InMaxCoins;
	MaxObstacles = InMaxObstacles;
	MaxTrackPieces = InMaxTrackPieces;
	RunStartTime = FDateTime::Now();
	
	SeededRandomStream.Initialize(TrackSeed);
	
	bSpawnSpecialCollectibles = false;
	
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ACollectibleCoin> ActorIterator(World); ActorIterator; ++ActorIterator)
			if (IsValid(*ActorIterator)) (*ActorIterator)->Destroy();
		for (TActorIterator<AMultiCollectible> ActorIterator(World); ActorIterator; ++ActorIterator)
			if (IsValid(*ActorIterator)) (*ActorIterator)->Destroy();
		for (TActorIterator<APowerUp> ActorIterator(World); ActorIterator; ++ActorIterator)
			if (IsValid(*ActorIterator)) (*ActorIterator)->Destroy();
		for (TActorIterator<AObstacle> ActorIterator(World); ActorIterator; ++ActorIterator)
			if (IsValid(*ActorIterator)) (*ActorIterator)->Destroy();
	}

	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController) return;

	ARabbitCharacter* Player = Cast<ARabbitCharacter>(PlayerController->GetPawn());
	if (!Player)
	{
		if (DefaultPawnClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Player = World->SpawnActor<ARabbitCharacter>(DefaultPawnClass, PlayerSpawnLocation, PlayerSpawnRotation, SpawnParams);
			if (Player) PlayerController->Possess(Player);
			}
		}
		else
		{
		Player->ResetRagdollState();
		FVector ResetLocation = FVector(100.0f, 0.0f, 200.0f);
		
		if (USkeletalMeshComponent* MeshComp = Player->GetMesh())
		{
			ARabbitCharacter* CDO = Player->GetClass()->GetDefaultObject<ARabbitCharacter>();
			if (CDO && CDO->GetMesh())
			{
				MeshComp->SetRelativeLocationAndRotation(CDO->GetMesh()->GetRelativeLocation(), CDO->GetMesh()->GetRelativeRotation());
				MeshComp->SetRelativeScale3D(CDO->GetMesh()->GetRelativeScale3D());
			}
			MeshComp->UpdateComponentToWorld();
		}
		
		FRotator ResetRotation = FRotator::ZeroRotator;
		Player->SetActorRotation(ResetRotation);
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) PC->SetControlRotation(ResetRotation);
		Player->SetActorLocation(ResetLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Player->ResetLanePosition();
		if (USceneComponent* RootComp = Player->GetRootComponent()) RootComp->UpdateComponentToWorld();
		
		PlayerSpawnLocation = ResetLocation;
		PlayerSpawnRotation = ResetRotation;
		DistanceTraveled = 0.0f;
		PreviousDistanceForScore = 0.0f;
		
		if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
		{
			MovementComp->SetMovementMode(MOVE_Walking);
			MovementComp->Velocity = FVector(1.0f, 0.0f, 0.0f) * Player->GetForwardSpeed();
			MovementComp->UpdateComponentVelocity();
			Player->SetActorTickEnabled(true);
			if (URabbitMovementComponent* RabbitMovement = Cast<URabbitMovementComponent>(MovementComp)) RabbitMovement->SetForwardSpeed(Player->GetForwardSpeed());
			MovementComp->MaxWalkSpeed = Player->GetForwardSpeed();
		}
	}

	CachedPlayer = Player;

	if (Player)
	{
		PlayerSpawnLocation = Player->GetActorLocation();
		PlayerSpawnRotation = Player->GetActorRotation();
		Player->ResetGASEffects();
		if (URabbitJumpComponent* JumpComp = Player->GetJumpComponent()) JumpComp->ResetJumpCount();
		if (Player->GetAttributeSet())
		{
			Player->GetAttributeSet()->SetBaseLives(static_cast<float>(StartingLives));
			Lives = StartingLives;
			float BaseSpeed = Player->GetAttributeSet()->GetBaseSpeed();
			Player->SetForwardSpeed(BaseSpeed);
			if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
			{
				MovementComp->MaxWalkSpeed = BaseSpeed;
				if (URabbitMovementComponent* RabbitMovement = Cast<URabbitMovementComponent>(MovementComp)) RabbitMovement->SetForwardSpeed(BaseSpeed);
				MovementComp->Velocity = FVector(1.0f, 0.0f, 0.0f) * BaseSpeed;
				MovementComp->UpdateComponentVelocity();
			}
		}
		else Lives = StartingLives;
	}

	if (Player && PlayerController)
	{
		if (PlayerController->GetPawn() != Player) PlayerController->Possess(Player);
	}
	
	PlayerController->SetPause(false);
	if (UWorld* WorldPtr = GetWorld()) WorldPtr->GetWorldSettings()->SetPauserPlayerState(nullptr);
	PlayerController->bShowMouseCursor = false;
	PlayerController->SetInputMode(FInputModeGameOnly());
	
	if (DeathPlayerController == PlayerController)
	{
		PlayerController->EnableInput(PlayerController);
		DeathPlayerController = nullptr;
	}
	
	if (GEngine && GEngine->GameViewport) FSlateApplication::Get().SetAllUserFocusToGameViewport();
	
	if (Player)
	{
		Player->SetActorTickEnabled(false);
		Player->PrimaryActorTick.bCanEverTick = true;
		Player->PrimaryActorTick.TickInterval = 0.0f;
		Player->PrimaryActorTick.TickGroup = TG_PrePhysics;
		Player->PrimaryActorTick.bStartWithTickEnabled = true;
		Player->SetActorTickEnabled(true);
		ApplyClassPerks(Player);
		Player->SetInvincible(true, 10.0f);
		
		if (UCharacterMovementComponent* MovementComp = Player->GetCharacterMovement())
		{
			MovementComp->SetActive(true);
			MovementComp->SetComponentTickEnabled(true);
			MovementComp->PrimaryComponentTick.bCanEverTick = true;
			MovementComp->Velocity = FVector(1.0f, 0.0f, 0.0f) * Player->GetForwardSpeed();
			MovementComp->SetMovementMode(MOVE_Walking);
			MovementComp->UpdateComponentVelocity();
		}
	}
	
	if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
	{
			SetGameState(EGameState::Playing);
		HUD->ShowInGameHUD();
		PlayerController->SetPause(false);
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputMode(FInputModeGameOnly());
	}

	if (TrackGenerator)
	{
		TrackGenerator->Reset();
		if (Player) TrackGenerator->Initialize(Player);
	}

	if (GameplayManager) GameplayManager->Reset();
}

void AEndlessRunnerGameMode::PauseGame()
{
	if (RunnerGameState == EGameState::Playing)
	{
		SetGameState(EGameState::Paused);
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
				PlayerController->SetPause(true);
		}
	}
}

void AEndlessRunnerGameMode::ResumeGame()
{
	if (RunnerGameState == EGameState::Paused)
	{
		SetGameState(EGameState::Playing);
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PlayerController = World->GetFirstPlayerController())
				PlayerController->SetPause(false);
		}
	}
}

void AEndlessRunnerGameMode::EndGame()
{
	if (RunnerGameState == EGameState::GameOver) return;

	// Finalize distance before state change
	if (CachedPlayer)
	{
		DistanceTraveled = TotalPreviousDistance + CachedPlayer->GetActorLocation().X;
	}

	SetGameState(EGameState::GameOver);
	
	if (!SeedId.IsEmpty() && WebServerInterface)
	{
		int32 DistanceMeters = FMath::RoundToInt(DistanceTraveled / 800.0f);
		int32 DurationSeconds = FMath::RoundToInt(GameTime);
		int32 TrackPiecesSpawned = TrackGenerator ? TrackGenerator->GetTotalTrackPiecesSpawned() : 0;
		FString StartedAtStr = RunStartTime.ToIso8601();
		
		// Collect actual sequence of pieces spawned
		TArray<FString> ActualSequence;
		if (TrackGenerator)
		{
			ActualSequence = TrackGenerator->GetCurrentPieceIds();
		}

		WebServerInterface->SubmitRun(
			SeedId, Score, DistanceMeters, DurationSeconds, RunCurrency, ObstaclesHit, PowerupsUsed, TrackPiecesSpawned,
			StartedAtStr, SelectedTrackIndices, false, bIsEndlessMode, ActualSequence
		);
	}
	
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			PlayerController->SetPause(true);
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputMode(FInputModeUIOnly());
			if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD()))
				HUD->ShowGameOverScreen();
		}
	}
}

void AEndlessRunnerGameMode::AddScore(int32 Points)
{
	ARabbitCharacter* Player = GetCachedPlayer();
	float EffectiveScoreMultiplier = (Player && Player->GetAttributeSet()) ? Player->GetScoreMultiplier() : 1.0f;
	if (EffectiveScoreMultiplier != 1.0f) Points = FMath::FloorToInt(Points * EffectiveScoreMultiplier);
	
	if (IsOnLastLegs())
	{
		float EffectiveMultiplier = LastLegsScoreMultiplier;
		FPlayerClassData ClassData = GetClassData(SelectedClass);
		if (ClassData.bScalesLastLegsWithSpeed && Player && Player->GetAttributeSet())
			EffectiveMultiplier = LastLegsScoreMultiplier * (1.0f + Player->GetAttributeSet()->GetSpeedMultiplier());
		Points = FMath::FloorToInt(Points * EffectiveMultiplier);
	}
	
	Score += Points;
}

void AEndlessRunnerGameMode::UpdateScore(float DeltaTime)
{
	float DistanceDelta = DistanceTraveled - PreviousDistanceForScore;
	if (DistanceDelta > 0.0f)
	{
		PreviousDistanceForScore = DistanceTraveled;
		float DistanceDeltaMeters = DistanceDelta / 800.0f;
		int32 PointsToAdd = FMath::FloorToInt(DistanceDeltaMeters * PointsPerMeter);
		if (PointsToAdd > 0) AddScore(PointsToAdd);
	}
}

void AEndlessRunnerGameMode::SetupEnhancedInput()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (!PlayerController) return;
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer) return;
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem) return;
	UInputMappingContext* IMC_Keyboard = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/EndlessRunner/Input/IMC_Keyboard"));
	if (IMC_Keyboard) InputSubsystem->AddMappingContext(IMC_Keyboard, 0);
}

void AEndlessRunnerGameMode::OnPlayerHitObstacle(int32 LivesLost, bool bInstantDeath)
{
	if (RunnerGameState != EGameState::Playing) return;
	ARabbitCharacter* Player = GetCachedPlayer();
	if (!Player) return;

	if (bInstantDeath)
	{
		if (Player)
		{
			FVector LaunchVelocity = Player->GetActorForwardVector() * 2000.0f + FVector(0.0f, 0.0f, 2500.0f);
			Player->EnableRagdollDeath(LaunchVelocity);
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) PC->DisableInput(PC);
		}
		if (Player->GetAttributeSet()) Player->GetAttributeSet()->SetBaseLives(0.0f);
		Lives = 0;
		EndGame();
		return;
	}

	FPlayerClassData ClassData = GetClassData(SelectedClass);
	float ActualLivesLost = static_cast<float>(LivesLost) * (ClassData.DamageReduction < 1.0f ? ClassData.DamageReduction : 1.0f);
	int32 RoundedLivesLost = FMath::Max(1, FMath::RoundToInt(ActualLivesLost));
	int32 CurrentLives = GetLives();
	bool bWasOnLastLegs = (CurrentLives <= 0);
	int32 NewLives = FMath::Max(0, CurrentLives - RoundedLivesLost);
	
	if (Player->GetAttributeSet()) Player->GetAttributeSet()->SetBaseLives(static_cast<float>(NewLives));
	Lives = NewLives;

	if (bWasOnLastLegs)
	{
		if (Player)
		{
			FVector LaunchVelocity = Player->GetActorForwardVector() * 2000.0f + FVector(0.0f, 0.0f, 2500.0f);
			Player->EnableRagdollDeath(LaunchVelocity);
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) PC->DisableInput(PC);
		}
		EndGame();
	}
}

void AEndlessRunnerGameMode::RespawnPlayer()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;
	FVector SpawnLocation = PlayerSpawnLocation;
	float GroundZ = 0.0f;
	bool bFoundGround = false;
	
	if (UWorld* World = GetWorld())
	{
		if (TrackGenerator)
		{
			TArray<ATrackPiece*> ActivePieces = TrackGenerator->GetActiveTrackPieces();
			float MinDist = FLT_MAX;
			ATrackPiece* Nearest = nullptr;
			for (ATrackPiece* Piece : ActivePieces)
			{
				if (Piece && IsValid(Piece))
				{
					float Dist = FMath::Abs(Piece->GetActorLocation().X - PlayerSpawnLocation.X);
					if (Dist < MinDist) { MinDist = Dist; Nearest = Piece; }
				}
			}
			if (Nearest) { GroundZ = Nearest->GetActorLocation().Z; bFoundGround = true; }
		}
		
		if (!bFoundGround)
		{
			FHitResult Hit;
			FCollisionObjectQueryParams Params;
			Params.AddObjectTypesToQuery(ECC_WorldStatic); Params.AddObjectTypesToQuery(ECC_WorldDynamic); Params.AddObjectTypesToQuery(ECC_Pawn);
			if (World->LineTraceSingleByObjectType(Hit, PlayerSpawnLocation + FVector(0,0,1000), PlayerSpawnLocation - FVector(0,0,1000), Params))
			{ GroundZ = Hit.ImpactPoint.Z; bFoundGround = true; }
		}
		SpawnLocation.Z = GroundZ + 200.0f;
	}

	ARabbitCharacter* Player = Cast<ARabbitCharacter>(PC->GetPawn());
	if (!Player)
	{
		if (DefaultPawnClass)
		{
			Player = GetWorld()->SpawnActor<ARabbitCharacter>(DefaultPawnClass, SpawnLocation, PlayerSpawnRotation);
			if (Player) PC->Possess(Player);
		}
	}
	else
	{
		Player->SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
		Player->SetActorRotation(PlayerSpawnRotation);
	}

	CachedPlayer = Player;
	if (Player)
	{
		Player->ResetGASEffects();
		if (UCharacterMovementComponent* MC = Player->GetCharacterMovement())
		{ MC->Velocity = FVector::ZeroVector; MC->SetMovementMode(MOVE_Walking); MC->UpdateComponentVelocity(); }
		Player->SetInvincible(true, 2.0f);
	}
	if (TrackGenerator && Player) TrackGenerator->UpdatePlayerReference(Player);
}

float AEndlessRunnerGameMode::GetPlayerSpeed() const
{
	ARabbitCharacter* Player = const_cast<AEndlessRunnerGameMode*>(this)->GetCachedPlayer();
	return (Player && Player->GetCharacterMovement()) ? Player->GetCharacterMovement()->Velocity.Size() : 0.0f;
}

int32 AEndlessRunnerGameMode::GetMaxJumpCount() const
{
	ARabbitCharacter* Player = const_cast<AEndlessRunnerGameMode*>(this)->GetCachedPlayer();
	return Player ? Player->GetCurrentMaxJumpCount() : 1;
}

int32 AEndlessRunnerGameMode::GetCurrentJumpCount() const
{
	ARabbitCharacter* Player = const_cast<AEndlessRunnerGameMode*>(this)->GetCachedPlayer();
	return (Player && Player->GetJumpComponent()) ? Player->GetJumpComponent()->GetCurrentJumpCount() : 0;
}

FString AEndlessRunnerGameMode::GetActivePowerUpStatus() const
{
	TArray<FString> Effects;
	ARabbitCharacter* Player = GetCachedPlayer();
	if (Player && Player->GetAttributeSet())
	{
		float S = 1.0f + Player->GetAttributeSet()->GetSpeedMultiplier();
		if (S != 1.0f) Effects.Add(FString::Printf(TEXT("Speed x%.1f"), S));
		float C = Player->GetCoinMultiplier();
		if (C != 1.0f) Effects.Add(FString::Printf(TEXT("Coins x%.1f"), C));
		float Sc = Player->GetScoreMultiplier();
		if (Sc != 1.0f) Effects.Add(FString::Printf(TEXT("Score x%.1f"), Sc));
	}
	if (CachedPlayer && CachedPlayer->IsInvincible()) Effects.Add(TEXT("Invincible"));
	if (bMagnetActive) Effects.Add(TEXT("Magnet"));
	if (bAutopilotActive) Effects.Add(TEXT("Autopilot"));
	return FString::Join(Effects, TEXT(" | "));
}

void AEndlessRunnerGameMode::AddRunCurrency(int32 Amount)
{
	ARabbitCharacter* Player = GetCachedPlayer();
	if (Player && Player->GetAttributeSet()) Amount = FMath::FloorToInt(Amount * Player->GetCoinMultiplier());
	RunCurrency += Amount;
	TrackCurrency += Amount;
}

void AEndlessRunnerGameMode::ClearPowerUpInvincibility() { GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle); }
void AEndlessRunnerGameMode::ClearGameModeInvincibility() { GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle); ARabbitCharacter* P = GetCachedPlayer(); if (P) P->SetInvincible(false); }

ARabbitCharacter* AEndlessRunnerGameMode::GetCachedPlayer() const
{
	if (CachedPlayer && IsValid(CachedPlayer)) return CachedPlayer;
	if (UWorld* W = GetWorld()) if (APlayerController* PC = W->GetFirstPlayerController()) return Cast<ARabbitCharacter>(PC->GetPawn());
	return nullptr;
}

void AEndlessRunnerGameMode::UpdateMagnetEffect(float DeltaTime)
{
	ARabbitCharacter* Player = GetCachedPlayer();
	if (!Player) return;
	FVector PL = Player->GetActorLocation();
	float MS = MagnetSpeed + Player->GetForwardSpeed();
	UWorld* W = GetWorld();
	if (!W) return;

	for (TActorIterator<ACollectibleCoin> CI(W); CI; ++CI)
	{
		ACollectibleCoin* C = *CI;
		if (!C || !IsValid(C) || C->IsCollected() || !C->IsMagnetable()) continue;
		if (FVector::Dist(PL, C->GetActorLocation()) <= MagnetRange)
		{
			FVector NL = C->GetActorLocation() + (PL - C->GetActorLocation()).GetSafeNormal() * MS * DeltaTime;
			C->SetActorLocation(NL, false, nullptr, ETeleportType::None);
			if (FVector::Dist(PL, NL) <= 20.0f && !C->IsCollected()) C->Collect();
			else if (USphereComponent* CS = C->GetCollisionSphere()) CS->UpdateOverlaps();
		}
	}

	for (TActorIterator<AMultiCollectible> MI(W); MI; ++MI)
	{
		AMultiCollectible* MC = *MI;
		if (!MC || !IsValid(MC) || !MC->IsMagnetable()) continue;
		const TArray<FCollectibleItem>& Items = MC->GetCollectibleItems();
		for (int32 i = 0; i < Items.Num(); ++i)
		{
			const FCollectibleItem& Item = Items[i];
			if (Item.bCollected || !Item.MeshComponent) continue;
			FVector IL = Item.MeshComponent->GetComponentLocation();
			if (FVector::Dist(PL, IL) <= MagnetRange)
			{
				FVector NWL = IL + (PL - IL).GetSafeNormal() * MS * DeltaTime;
				MC->CollectItem(i); // Simplified
			}
		}
	}
}

void AEndlessRunnerGameMode::ClearMagnet() { bMagnetActive = false; GetWorldTimerManager().ClearTimer(MagnetTimerHandle); }
void AEndlessRunnerGameMode::ClearAutopilot() { bAutopilotActive = false; if (ARabbitCharacter* P = GetCachedPlayer()) P->SetAutopilot(false); GetWorldTimerManager().ClearTimer(AutopilotTimerHandle); }

FPlayerClassData AEndlessRunnerGameMode::GetClassData(EPlayerClass Class) const { if (UPlayerClassDefinition* D = GetClassDefinition(Class)) return D->ClassData; return FPlayerClassData(); }
UPlayerClassDefinition* AEndlessRunnerGameMode::GetClassDefinition(EPlayerClass Class) const { for (UPlayerClassDefinition* D : PlayerClassDefinitions) if (D && D->ClassType == Class) return D; return nullptr; }
UPlayerClassDefinition* AEndlessRunnerGameMode::GetSelectedClassDefinition() const { return GetClassDefinition(SelectedClass); }

void AEndlessRunnerGameMode::ApplyClassPerks(ARabbitCharacter* Player)
{
	if (!Player) return;
	FPlayerClassData D = GetClassData(SelectedClass);
	if (D.ExtraLives > 0) { float L = static_cast<float>(StartingLives + D.ExtraLives); if (Player->GetAttributeSet()) Player->GetAttributeSet()->SetBaseLives(L); Lives = StartingLives + D.ExtraLives; }
	Player->SetMaxJumpCount(D.MaxJumpCount);
	if (URabbitJumpComponent* J = Player->GetJumpComponent()) J->ResetJumpCount();
	if (D.CapsuleHalfHeight > 0.0f) if (UCapsuleComponent* C = Player->GetCapsuleComponent()) Player->SetCapsuleSize(D.CapsuleHalfHeight, C->GetUnscaledCapsuleRadius());
	if (D.bNeverNeedsCrouch) Player->SetNeverNeedsCrouch(true);
	if (D.bCanBreakObstacles) Player->SetCanBreakObstacles(true);
	if (D.bSpawnsSpecialCollectibles) bSpawnSpecialCollectibles = true;
	if (D.bHasStartingMagnet && D.StartingMagnetDuration > 0.0f) { bMagnetActive = true; GetWorldTimerManager().SetTimer(MagnetTimerHandle, this, &AEndlessRunnerGameMode::ClearMagnet, D.StartingMagnetDuration, false); }
	if (D.BaseSpeedMultiplier != 1.0f) Player->SetForwardSpeed(Player->GetForwardSpeed() * D.BaseSpeedMultiplier);
	
	// Sync responsiveness for dynamic lane change throttle
	Player->SetLaneChangeResponsiveness(D.LaneChangeResponsiveness);

	// Apply new movement stats
	if (URabbitAttributeSet* AS = Player->GetAttributeSet())
	{
		AS->SetBaseJumpHeight(D.BaseJumpHeight);
		AS->SetJumpHeightMultiplier(D.JumpHeightMultiplier);
		AS->SetBaseMultiJumpHeight(D.BaseMultiJumpHeight);
		AS->SetMultiJumpHeightMultiplier(D.MultiJumpHeightMultiplier);
		AS->SetBaseGravityScale(D.BaseGravityScale);
		AS->SetGravityScaleMultiplier(D.GravityScaleMultiplier);
		AS->SetBaseLaneTransitionSpeed(D.BaseLaneTransitionSpeed);
		AS->SetLaneTransitionSpeedMultiplier(D.LaneTransitionSpeedMultiplier);
	}
}

int32 AEndlessRunnerGameMode::GetLives() const { const ARabbitCharacter* P = GetCachedPlayer(); return (P && P->GetAttributeSet()) ? FMath::RoundToInt(P->GetAttributeSet()->GetCurrentLives()) : Lives; }
bool AEndlessRunnerGameMode::IsOnLastLegs() const { return GetLives() <= 0; }

void AEndlessRunnerGameMode::SetTrackSeed(int32 NewSeed) { TrackSeed = FMath::Abs(NewSeed); SeededRandomStream.Initialize(TrackSeed); }
void AEndlessRunnerGameMode::GenerateRandomSeed() { TrackSeed = FMath::RandRange(1, 2147483647); SeededRandomStream.Initialize(TrackSeed); }

void AEndlessRunnerGameMode::OnSeedReceived(const FRunSeedData& SeedData)
{
	TrackSeed = SeedData.Seed; SeedId = SeedData.SeedId; MaxCoins = SeedData.MaxCoins; MaxObstacles = SeedData.MaxObstacles; MaxTrackPieces = SeedData.MaxTrackPieces; SeededRandomStream.Initialize(TrackSeed);
}

void AEndlessRunnerGameMode::OnTrackSelectionReceived(const FTrackSelectionData& SelectionData)
{
	CurrentTrackSelection = SelectionData; CurrentTier = SelectionData.Tier;
	if (CurrentTier == 1) SelectedTrackIndices.Empty();
	ShowTrackSelection();
}

void AEndlessRunnerGameMode::OnPowerUpUsed() { if (RunnerGameState == EGameState::Playing) PowerupsUsed++; }
void AEndlessRunnerGameMode::OnSeedRequestError(const FString& M) { TrackSeed = FMath::RandRange(1, 2147483647); SeedId = TEXT(""); SeededRandomStream.Initialize(TrackSeed); SetGameState(EGameState::Menu); }

void AEndlessRunnerGameMode::OnTrackSequenceReceived(const FTrackSequenceData& SequenceData)
{
	UE_LOG(LogTemp, Warning, TEXT("GameMode: OnTrackSequenceReceived called. Pieces: %d, CurrentTier: %d"), SequenceData.Pieces.Num(), CurrentTier);
	TrackSequence = SequenceData;
	bTrackSequenceLoaded = true;
	
	CurrentShopIndex = -1;
	LastVisitedShopPiece = nullptr;

	if (CurrentTier > 1) { TotalPreviousDistance = DistanceTraveled; PreviousDistanceForScore = DistanceTraveled; }
	else { Score = 0; RunCurrency = 0; TrackCurrency = 0; ObstaclesHit = 0; PowerupsUsed = 0; DistanceTraveled = 0.0f; TotalPreviousDistance = 0.0f; PreviousDistanceForScore = 0.0f; GameTime = 0.0f; RunStartTime = FDateTime::Now(); }
	
	SetGameState(EGameState::Playing);
	if (TrackSeed > 0) SeededRandomStream.Initialize(TrackSeed);
	
	UWorld* World = GetWorld(); if (!World) return;
	APlayerController* PlayerController = World->GetFirstPlayerController(); if (!PlayerController) return;
	
	ARabbitCharacter* Player = Cast<ARabbitCharacter>(PlayerController->GetPawn());
	if (!Player) { if (DefaultPawnClass) { Player = World->SpawnActor<ARabbitCharacter>(DefaultPawnClass, PlayerSpawnLocation, PlayerSpawnRotation); if (Player) PlayerController->Possess(Player); } }
	else { Player->ResetRagdollState(); FVector RL(100.0f, 0.0f, 400.0f); Player->SetActorLocation(RL, false, nullptr, ETeleportType::TeleportPhysics); Player->SetActorRotation(FRotator::ZeroRotator); Player->ResetLanePosition(); PlayerSpawnLocation = RL; PlayerSpawnRotation = FRotator::ZeroRotator; }
	
	CachedPlayer = Player;
	if (Player)
	{
		if (CurrentTier == 1) { Player->ResetGASEffects(); if (URabbitJumpComponent* J = Player->GetJumpComponent()) J->ResetJumpCount(); if (Player->GetAttributeSet()) { Player->GetAttributeSet()->SetBaseLives(static_cast<float>(StartingLives)); Lives = StartingLives; } ApplyClassPerks(Player); }
		if (Player->GetAttributeSet()) { float BS = Player->GetAttributeSet()->GetBaseSpeed(); Player->SetForwardSpeed(BS); if (UCharacterMovementComponent* MC = Player->GetCharacterMovement()) { MC->MaxWalkSpeed = BS; if (URabbitMovementComponent* RM = Cast<URabbitMovementComponent>(MC)) RM->SetForwardSpeed(BS); MC->Velocity = FVector(1,0,0) * BS; MC->UpdateComponentVelocity(); } }
		Player->SetInvincible(true, 10.0f);
	}
	
	if (TrackGenerator) { 
		TrackGenerator->LoadTrackSequence(SequenceData); 
		TrackGenerator->UpdatePlayerReference(Player); 
		TrackGenerator->Initialize(Player); // Start spawning from sequence
	}
	if (GameplayManager && CurrentTier == 1) GameplayManager->Reset();
	
	PlayerController->SetPause(false); PlayerController->bShowMouseCursor = false; PlayerController->SetInputMode(FInputModeGameOnly());
	if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PlayerController->GetHUD())) HUD->ShowInGameHUD();
	if (GEngine && GEngine->GameViewport) FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void AEndlessRunnerGameMode::OnShopItemsReceived(const FShopData& ShopData) 
{ 
	ShopItems = ShopData; 
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(PC->GetHUD()))
		{
			HUD->OnShopItemsReceived(ShopData);
		}
	}
}
void AEndlessRunnerGameMode::OnBossRewardsReceived(const TArray<FBossRewardData>& Rewards) { BossRewards = Rewards; if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())) HUD->ShowBossRewards(); }

void AEndlessRunnerGameMode::ShowTrackSelection() { SetGameState(EGameState::TrackSelection); if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())) HUD->ShowTrackSelection(); }
void AEndlessRunnerGameMode::SelectTrack(int32 TrackIndex) { if (TrackIndex < 0 || TrackIndex >= CurrentTrackSelection.Tracks.Num()) return; CurrentTrackIndex = TrackIndex; SelectedTrackIndices.Add(TrackIndex); if (WebServerInterface && !SeedId.IsEmpty()) WebServerInterface->SelectTrack(SeedId, CurrentTier, TrackIndex); }

void AEndlessRunnerGameMode::OnShopTriggerOverlap(ATrackPiece* ShopPiece) 
{ 
	if (!ShopPiece || RunnerGameState != EGameState::Playing) return; 

	if (ShopPiece != LastVisitedShopPiece)
	{
		LastVisitedShopPiece = ShopPiece;
		CurrentShopIndex++;
		EnterShop(CurrentShopIndex);
	}
}
void AEndlessRunnerGameMode::OnBossTriggerOverlap(ATrackPiece* P) { if (!P || RunnerGameState != EGameState::Playing) return; OnBossReached(); }
void AEndlessRunnerGameMode::OnBossEndTriggerOverlap(ATrackPiece* P) { if (!P || RunnerGameState != EGameState::BossEncounter) return; OnBossDefeated(); }

void AEndlessRunnerGameMode::EnterShop(int32 Index) 
{ 
	CurrentShopIndex = Index; 
	SetGameState(EGameState::Shop); 
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
	{
		PC->SetPause(true);
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	if (TrackSequence.AllShopsData.IsValidIndex(Index))
	{
		OnShopItemsReceived(TrackSequence.AllShopsData[Index]);
	}
	else if (WebServerInterface && !SeedId.IsEmpty()) 
	{
		WebServerInterface->GetShopItems(SeedId, CurrentTier, CurrentTrackIndex, Index); 
	}

	if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())) HUD->ShowShop(); 
}

void AEndlessRunnerGameMode::ExitShop() { CurrentShopIndex = -1; SetGameState(EGameState::Playing); if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) { PC->SetPause(false); PC->bShowMouseCursor = false; PC->SetInputMode(FInputModeGameOnly()); } if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())) HUD->ShowInGameHUD(); }

void AEndlessRunnerGameMode::PurchaseItem(const FString& ItemId)
{
	// Find item in current shop list to get the cost
	int32 Cost = 0;
	FShopItemData SelectedItem;
	bool bFound = false;

	for (const FShopItemData& Item : ShopItems.Items)
	{
		if (Item.Id == ItemId)
		{
			Cost = Item.Cost;
			SelectedItem = Item;
			bFound = true;
			break;
		}
	}

	if (bFound && RunCurrency >= Cost)
	{
		if (WebServerInterface && !SeedId.IsEmpty())
		{
			WebServerInterface->PurchaseItem(ItemId, Cost);
		}

		// Subtract locally for immediate feedback
		RunCurrency -= Cost;
		
		// Apply effect
		if (ContentRegistry)
		{
			// Check if it's a direct PowerUp
			UPowerUpDefinition* PD = ContentRegistry->FindPowerUpById(ItemId);
			if (PD && PD->PowerUpClass)
			{
				if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
				{
					if (ARabbitCharacter* Player = Cast<ARabbitCharacter>(PC->GetPawn()))
					{
						FActorSpawnParameters SP; 
						SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						APowerUp* SpawnedPU = GetWorld()->SpawnActor<APowerUp>(PD->PowerUpClass, Player->GetActorLocation(), FRotator::ZeroRotator, SP);
						if (SpawnedPU)
						{
							SpawnedPU->Collect(Player);
						}
					}
				}
			}
			else
			{
				// Check if it's a Shop Item Definition
				UShopItemDefinition* SID = ContentRegistry->FindShopItemById(ItemId);
				if (SID)
				{
					if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
					{
						if (ARabbitCharacter* Player = Cast<ARabbitCharacter>(PC->GetPawn()))
						{
							// Apply based on tag and value
							if (SID->EffectTag.IsValid())
							{
								Player->ApplyEffectByTag(SID->EffectTag, SID->EffectValue);
								UE_LOG(LogTemp, Log, TEXT("GameMode: Applied shop item effect: %s (Value: %.2f)"), *SID->EffectTag.ToString(), SID->EffectValue);
							}
						}
					}
				}
			}
		}
	}
}

void AEndlessRunnerGameMode::RerollShop(int32 Index) { TArray<int32> RC = {50, 100, 150, 200}; int32 Cost = ShopItems.Items.Num() < RC.Num() ? RC[ShopItems.Items.Num()] : RC.Last(); if (TrackCurrency < Cost) return; TrackCurrency -= Cost; if (WebServerInterface && !SeedId.IsEmpty()) WebServerInterface->RerollShop(SeedId, CurrentTier, CurrentTrackIndex, Index); }

void AEndlessRunnerGameMode::OnBossReached() { SetGameState(EGameState::BossEncounter); }
void AEndlessRunnerGameMode::OnBossDefeated() 
{ 
	SetGameState(EGameState::BossReward); 
	
	if (TrackSequence.BossRewards.Num() > 0)
	{
		OnBossRewardsReceived(TrackSequence.BossRewards);
	}
	else if (WebServerInterface && !SeedId.IsEmpty()) 
	{
		WebServerInterface->GetBossRewards(SeedId, CurrentTier); 
	}
}

void AEndlessRunnerGameMode::SelectBossReward(const FString& ID)
{
	UPowerUpDefinition* D = FindPowerUpDefinitionById(ID);
	if (D) { ARabbitCharacter* P = Cast<ARabbitCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()); if (P && D->PowerUpClass) { FActorSpawnParameters SP; SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; APowerUp* A = GetWorld()->SpawnActor<APowerUp>(D->PowerUpClass, P->GetActorLocation(), FRotator::ZeroRotator, SP); if (A) A->Collect(P); } }
	if (CurrentTier >= 3) CompleteRun(); else AdvanceToNextTier();
}

void AEndlessRunnerGameMode::AdvanceToNextTier() { CurrentTier++; CurrentTrackIndex = 0; TrackSequence.Pieces.Empty(); TrackSequence.ShopPositions.Empty(); TrackSequence.BossId = TEXT(""); if (WebServerInterface && !SeedId.IsEmpty()) WebServerInterface->RequestTierTracks(SeedId, CurrentTier); }
void AEndlessRunnerGameMode::CompleteRun() 
{ 
	if (WebServerInterface && !SeedId.IsEmpty()) 
	{
		TArray<FString> PieceIds;
		for (const FTrackPiecePrescription& P : TrackSequence.Pieces) PieceIds.Add(P.PieceId);

		WebServerInterface->SubmitRun(
			SeedId, Score, FMath::RoundToInt(GetDistanceTraveled()), FMath::RoundToInt(GameTime), 
			RunCurrency, ObstaclesHit, PowerupsUsed, 
			TrackGenerator ? TrackGenerator->GetTotalTrackPiecesSpawned() : 0, 
			RunStartTime.ToIso8601(), SelectedTrackIndices, true, false, PieceIds
		); 
	}
	SetGameState(EGameState::GameOver); 
	ShowEndlessModeOption(); 
}
void AEndlessRunnerGameMode::ShowEndlessModeOption() { if (AEndlessRunnerHUD* HUD = Cast<AEndlessRunnerHUD>(GetWorld()->GetFirstPlayerController()->GetHUD())) HUD->ShowEndlessModePrompt(); }
void AEndlessRunnerGameMode::StartEndlessMode() { bIsEndlessMode = true; bTrackSequenceLoaded = false; CurrentTier = 3; if (TrackGenerator) { TrackGenerator->SetCurrentDifficulty(3); TrackGenerator->SetEndlessMode(true); } if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) { PC->SetPause(false); PC->bShowMouseCursor = false; PC->SetInputMode(FInputModeGameOnly()); } SetGameState(EGameState::Playing); }

UPowerUpDefinition* AEndlessRunnerGameMode::FindPowerUpDefinitionById(const FString& ID) const
{
	if (ContentRegistry)
	{
		return ContentRegistry->FindPowerUpById(ID);
	}
	return nullptr;
}
