// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PlayerClass.h"
#include "WebServerInterface.h"
#include "ReplayModels.h"
#include "EndlessRunnerGameMode.generated.h"

class ATrackGenerator;
class UGameplayManager;
class UCurrencyManager;
class ARabbitCharacter;
class AEndlessRunnerHUD;
class UPlayerClassDefinition;
class UWebServerInterface;
class UPowerUpDefinition;
class UContentRegistry;

UENUM(BlueprintType)
enum class EGameState : uint8
{
	Menu			UMETA(DisplayName = "Menu"),
	TrackSelection	UMETA(DisplayName = "Track Selection"),
	Playing			UMETA(DisplayName = "Playing"),
	Shop			UMETA(DisplayName = "Shop"),
	BossEncounter	UMETA(DisplayName = "Boss Encounter"),
	BossReward		UMETA(DisplayName = "Boss Reward"),
	Paused			UMETA(DisplayName = "Paused"),
	GameOver		UMETA(DisplayName = "Game Over")
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

	/** Start game - requests seed from server first */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGame();

	/** Start game with seed (called after seed is received from server) */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void StartGameWithSeed(int32 Seed, const FString& InSeedId, int32 InMaxCoins, int32 InMaxObstacles, int32 InMaxTrackPieces);

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

	/** Purchase an item from the shop */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void PurchaseItem(const FString& ItemId);

	/** Get track generator */
	UFUNCTION(BlueprintPure, Category = "Game")
	ATrackGenerator* GetTrackGenerator() const { return TrackGenerator; }

	/** Get content registry */
	UFUNCTION(BlueprintPure, Category = "Game")
	UContentRegistry* GetContentRegistry() const { return ContentRegistry; }

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

	/** Get track currency (for shop) */
	UFUNCTION(BlueprintPure, Category = "Score")
	int32 GetTrackCurrency() const { return TrackCurrency; }

	/** Get current shop items */
	const FShopData& GetShopItems() const { return ShopItems; }

	/** Get current boss rewards */
	const TArray<FBossRewardData>& GetBossRewards() const { return BossRewards; }

	/** Get current track selection data */
	const FTrackSelectionData& GetCurrentTrackSelection() const { return CurrentTrackSelection; }

	/** Get current shop index */
	int32 GetCurrentShopIndex() const { return CurrentShopIndex; }

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

	/** Track powerup usage */
	UFUNCTION(BlueprintCallable, Category = "Game")
	void OnPowerUpUsed();

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

	/** Check if track sequence is loaded (seeded run) */
	UFUNCTION(BlueprintPure, Category = "Track Progression")
	bool IsTrackSequenceLoaded() const { return bTrackSequenceLoaded; }

	/** Handle seed received from server */
	UFUNCTION()
	void OnSeedReceived(const FRunSeedData& SeedData);

	/** Handle seed request error */
	UFUNCTION()
	void OnSeedRequestError(const FString& ErrorMessage);

	/** Handle track selection received from server */
	UFUNCTION()
	void OnTrackSelectionReceived(const FTrackSelectionData& SelectionData);

	/** Handle track sequence received from server */
	UFUNCTION()
	void OnTrackSequenceReceived(const FTrackSequenceData& SequenceData);

	/** Handle shop items received from server */
	UFUNCTION()
	void OnShopItemsReceived(const FShopData& ShopData);

	/** Handle boss rewards received from server */
	UFUNCTION()
	void OnBossRewardsReceived(const TArray<FBossRewardData>& Rewards);

	/** Show track selection UI for current tier */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void ShowTrackSelection();

	/** Select a track for the current tier */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void SelectTrack(int32 TrackIndex);

	/** Called when player enters a shop room trigger in BP */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Runner")
	void OnShopTriggerOverlap(ATrackPiece* ShopPiece);

	/** Called when player enters a boss room trigger in BP */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Runner")
	void OnBossTriggerOverlap(ATrackPiece* BossPiece);

	/** Called when player reaches the end of a boss room trigger in BP */
	UFUNCTION(BlueprintCallable, Category = "Dungeon Runner")
	void OnBossEndTriggerOverlap(ATrackPiece* BossPiece);

	/** Enter shop (pause game, show shop UI) */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void EnterShop(int32 ShopIndex);

	/** Exit shop (resume game) */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ExitShop();

	/** Reroll shop items */
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RerollShop(int32 ShopIndex);

	/** Called when boss piece is reached */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void OnBossReached();

	/** Called when boss is defeated */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void OnBossDefeated();

	/** Select a boss reward */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SelectBossReward(const FString& RewardId);

	/** Advance to next tier (after boss reward selection) */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void AdvanceToNextTier();

	/** Complete the run (after final boss) */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void CompleteRun();

	/** Show endless mode option (after final boss) */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void ShowEndlessModeOption();

	/** Start endless mode */
	UFUNCTION(BlueprintCallable, Category = "Track Progression")
	void StartEndlessMode();

	/** Replay System */
	UFUNCTION(BlueprintCallable, Category = "Replay")
	void StartReplay(int32 Seed, const FString& InSeedId, EPlayerClass PlayerClass, const TArray<FReplayEvent>& ReplayData);

	UFUNCTION(BlueprintPure, Category = "Replay")
	bool IsReplayMode() const { return bIsReplayMode; }

	/** Check if magnet is active */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsMagnetActive() const { return bMagnetActive; }

	/** Check if autopilot is active */
	UFUNCTION(BlueprintPure, Category = "PowerUp")
	bool IsAutopilotActive() const { return bAutopilotActive; }

	/** Find powerup definition by ID */
	UPowerUpDefinition* FindPowerUpDefinitionById(const FString& PowerUpId) const;

protected:
	/** Track generator reference */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	ATrackGenerator* TrackGenerator;

	/** Content registry */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UContentRegistry* ContentRegistry;

	/** Gameplay manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UGameplayManager* GameplayManager;

	/** Currency manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UCurrencyManager* CurrencyManager;

	/** Web server interface */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Managers")
	UWebServerInterface* WebServerInterface;

	/** Current game state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game")
	EGameState RunnerGameState = EGameState::Menu;

	/** Current score */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 Score = 0;

	/** Distance traveled (in game units) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	float DistanceTraveled = 0.0f;

	/** Distance from previously completed tracks in this run */
	float TotalPreviousDistance = 0.0f;

	/** Previous distance for score calculation */
	float PreviousDistanceForScore = 0.0f;

	/** Run currency (temporary, resets each game - collected during current run) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 RunCurrency = 0;

	/** Game time elapsed (in seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	float GameTime = 0.0f;

	/** Run statistics (for submission) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 ObstaclesHit = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Score")
	int32 PowerupsUsed = 0;

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

	/** Game over delay in seconds (to see ragdoll fly) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float GameOverDelay = 2.5f;

	/** Timer handle for game over delay */
	FTimerHandle GameOverDelayTimerHandle;

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

	/** Seed ID from server (for run submission) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	FString SeedId;

	/** Max values from server seed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	int32 MaxCoins = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	int32 MaxObstacles = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	int32 MaxTrackPieces = 0;

	/** Run start time (for submission) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Seed")
	FDateTime RunStartTime;

	/** Seeded random stream for deterministic generation */
	FRandomStream SeededRandomStream;

	/** Current tier (1, 2, or 3) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Progression")
	int32 CurrentTier = 1;

	/** Current track index within the tier (0, 1, or 2) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Progression")
	int32 CurrentTrackIndex = 0;

	/** Track currency (collected during current track, resets per track) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Progression")
	int32 TrackCurrency = 0;

	/** Current track sequence data (piece IDs, shop positions, boss ID) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Progression")
	FTrackSequenceData TrackSequence;

	/** Current shop index */
	int32 CurrentShopIndex = -1;

	/** Current shop items */
	FShopData ShopItems;

	/** Current boss rewards */
	TArray<FBossRewardData> BossRewards;

	/** Current track selection data from server */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track Progression")
	FTrackSelectionData CurrentTrackSelection;

	/** Selected track indices (one per tier) */
	TArray<int32> SelectedTrackIndices;

	/** Full sequence of piece IDs spawned in this run */
	TArray<FString> FullPieceSequence;

	/** Last shop piece visited to prevent double-triggering */
	UPROPERTY()
	ATrackPiece* LastVisitedShopPiece = nullptr;

	/** Flag indicating if track sequence is loaded (seeded run) */
	bool bTrackSequenceLoaded = false;

	/** Is the player in endless mode */
	bool bIsEndlessMode = false;

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

	/** Replay State */
	bool bIsReplayMode = false;
	TArray<FReplayEvent> CurrentReplayBuffer;
	int32 CurrentReplayEventIndex = 0;
	float ReplayStartTime = 0.0f;

	void UpdateReplay(float DeltaTime);

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
