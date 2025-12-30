// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerClass.h"
#include "EndlessRunnerGameMode.generated.h"

class ATrackGenerator;
class UGameplayManager;
class UCurrencyManager;
class ARabbitCharacter;
class AEndlessRunnerHUD;
class UPlayerClassDefinition;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Menu		UMETA(DisplayName = "Menu"),
	Playing		UMETA(DisplayName = "Playing"),
	Paused		UMETA(DisplayName = "Paused"),
	GameOver	UMETA(DisplayName = "Game Over")
};

/**
 * Main game mode for endless runner
 * Manages game state, scoring, and core gameplay systems
 */
UCLASS()
class SEWERSCUTTLE_API AEndlessRunnerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEndlessRunnerGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	/** Get current game state */
	UFUNCTION(BlueprintPure, Category = "Game")
	EGameState GetGameState() const { return RunnerGameState; }

	/** Set game state */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void SetGameState(EGameState NewState);

	/** Start game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGame();

	/** Pause game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void PauseGame();

	/** Resume game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ResumeGame();

	/** End game */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EndGame();

	/** Get current score */
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetScore() const { return Score; }

	/** Add to score */
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(int32 Points);

	/** Get distance traveled in meters (1 meter = 800 game units) */
	UFUNCTION(BlueprintPure, Category = "Score")
	float GetDistanceTraveled() const { return DistanceTraveled / 800.0f; }

	/** Get game time */
	UFUNCTION(BlueprintPure, Category = "Score")
	float GetGameTime() const { return GameTime; }

	/** Get track generator */
	UFUNCTION(BlueprintPure, Category = "Game")
	ATrackGenerator* GetTrackGenerator() const { return TrackGenerator; }

	/** Get gameplay manager */
	UFUNCTION(BlueprintPure, Category = "Game")
	UGameplayManager* GetGameplayManager() const { return GameplayManager; }

	/** Get currency manager (for persistent coins/wallet) */
	UFUNCTION(BlueprintPure, Category = "Game")
	UCurrencyManager* GetCurrencyManager() const { return CurrencyManager; }

	/** Get selected player class */
	UFUNCTION(BlueprintPure, Category = "Class")
	EPlayerClass GetSelectedClass() const { return SelectedClass; }

	/** Set selected player class */
	UFUNCTION(BlueprintCallable, Category = "Class")
	void SetSelectedClass(EPlayerClass Class) { SelectedClass = Class; }

	/** Get class data for a specific class enum */
	UFUNCTION(BlueprintPure, Category = "Class")
	FPlayerClassData GetClassData(EPlayerClass Class) const;

	/** Get class definition data asset for a class enum */
	UFUNCTION(BlueprintPure, Category = "Class")
	UPlayerClassDefinition* GetClassDefinition(EPlayerClass Class) const;

	/** Get selected class definition data asset */
	UFUNCTION(BlueprintPure, Category = "Class")
	UPlayerClassDefinition* GetSelectedClassDefinition() const;

	/** Get all available class definitions */
	UFUNCTION(BlueprintPure, Category = "Class")
	TArray<UPlayerClassDefinition*> GetAllClassDefinitions() const { return PlayerClassDefinitions; }

	/** Check if special collectibles should spawn (Collector class) */
	UFUNCTION(BlueprintPure, Category = "Class")
	bool ShouldSpawnSpecialCollectibles() const { return bSpawnSpecialCollectibles; }

	/** Get run currency (temporary, resets each game) */
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetRunCurrency() const { return RunCurrency; }

	/** Add to run currency (temporary, resets each game) */
	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddRunCurrency(int32 Amount);

	/** Get current lives */
	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetLives() const;

	/** Get if player is on last legs (0 lives) */
	UFUNCTION(BlueprintPure, Category = "Game")
	bool IsOnLastLegs() const;

	/** Handle player hit by obstacle */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPlayerHitObstacle(int32 LivesLost = 1, bool bInstantDeath = false);

	/** Respawn player */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void RespawnPlayer();

	/** Get player speed (for HUD) */
	UFUNCTION(BlueprintPure, Category = "Game")
	float GetPlayerSpeed() const;

	/** Get max jump count (for HUD) */
	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetMaxJumpCount() const;

	/** Get current jump count (jumps used in current air time, for HUD) */
	UFUNCTION(BlueprintPure, Category = "Game")
	int32 GetCurrentJumpCount() const;

	/** Get active powerup type (for HUD) */
	UFUNCTION(BlueprintPure, Category = "Game")
	FString GetActivePowerUpStatus() const;

	/** Get current track seed */
	UFUNCTION(BlueprintPure, Category = "Seed")
	int32 GetTrackSeed() const { return TrackSeed; }

	/** Set track seed (for deterministic generation) */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	void SetTrackSeed(int32 NewSeed);

	/** Generate a random seed */
	UFUNCTION(BlueprintCallable, Category = "Seed")
	void GenerateRandomSeed();

	/** Get seeded random stream (for use by TrackGenerator and SpawnManager) */
	FRandomStream& GetSeededRandomStream() { return SeededRandomStream; }

	/** Check if magnet is active */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsMagnetActive() const { return bMagnetActive; }

	/** Check if autopilot is active */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsAutopilotActive() const { return bAutopilotActive; }

protected:
	/** Track generator reference */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	ATrackGenerator* TrackGenerator;

	/** Gameplay manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UGameplayManager* GameplayManager;

	/** Currency manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UCurrencyManager* CurrencyManager;

	/** Current game state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	EGameState RunnerGameState = EGameState::Menu;

	/** Current score */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 Score = 0;

	/** Distance traveled (in game units) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	float DistanceTraveled = 0.0f;

	/** Previous distance for score calculation */
	float PreviousDistanceForScore = 0.0f;

	/** Run currency (temporary, resets each game - collected during current run) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 RunCurrency = 0;

	/** Game time elapsed (in seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	float GameTime = 0.0f;

	/** Points per meter of distance traveled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score", meta = (ClampMin = "0.1", ClampMax = "100.0"))
	float PointsPerMeter = 10.0f;

	/** Starting number of lives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "0", ClampMax = "10"))
	int32 StartingLives = 3;

	/** Current number of lives */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	int32 Lives = 3;

	/** Score multiplier when on last legs (0 lives) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float LastLegsScoreMultiplier = 2.0f;

	/** Respawn delay in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float RespawnDelay = 1.0f;

	/** Player spawn location (cached for respawn) */
	FVector PlayerSpawnLocation;

	/** Player spawn rotation (cached for respawn) */
	FRotator PlayerSpawnRotation;

	/** Selected player class enum (for compatibility) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Class")
	EPlayerClass SelectedClass = EPlayerClass::Vanilla;

	/** Player class definitions (data assets) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TArray<UPlayerClassDefinition*> PlayerClassDefinitions;

	/** Flag for spawning special collectibles (Collector class) */
	bool bSpawnSpecialCollectibles = false;

	/** Track generation seed (for deterministic/reproducible tracks) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	int32 TrackSeed = 0;

	/** Seeded random stream for deterministic generation */
	FRandomStream SeededRandomStream;

protected:
	/** Set up Enhanced Input Mapping Context */
	void SetupEnhancedInput();

	/** Update score based on distance */
	void UpdateScore(float DeltaTime);

	/** Update magnet effect (attracts collectibles) */
	void UpdateMagnetEffect(float DeltaTime);

	/** Apply class perks to player */
	void ApplyClassPerks(ARabbitCharacter* Player);

	/** Clear individual power-up effects */
	void ClearPowerUpInvincibility();
	void ClearMagnet();
	void ClearAutopilot();
	void ClearGameModeInvincibility();

	/** Get cached player reference */
	ARabbitCharacter* GetCachedPlayer() const;

	/** Magnet power-up state */
	bool bMagnetActive = false;

	/** Magnet attraction range (in game units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Magnet", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float MagnetRange = 800.0f;

	/** Magnet attraction speed (units per second, base speed - scales with player speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PowerUp|Magnet", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
	float MagnetSpeed = 1500.0f;

	/** Autopilot power-up state */
	bool bAutopilotActive = false;

private:
	/** Timer handle for respawn delay */
	FTimerHandle RespawnTimerHandle;

	/** Cached player reference (to avoid casting every frame) */
	ARabbitCharacter* CachedPlayer = nullptr;

	/** Player controller that had input disabled during ragdoll death (for re-enabling on restart) */
	APlayerController* DeathPlayerController = nullptr;

private:
	/** Timer handles for power-up effects */
	FTimerHandle InvincibilityTimerHandle;
	FTimerHandle MagnetTimerHandle;
	FTimerHandle AutopilotTimerHandle;
};

