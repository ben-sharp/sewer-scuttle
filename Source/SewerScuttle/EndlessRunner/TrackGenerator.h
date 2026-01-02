// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WebServerInterface.h"
#include "TrackGenerator.generated.h"

class ATrackPiece;
class UTrackPieceDefinition;
class ARabbitCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopPieceReached);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossPieceReached);

/**
 * Manages track generation (finite tracks with shops and bosses)
 * Spawns track pieces from a predefined sequence
 */
UCLASS()
class SEWERSCUTTLE_API ATrackGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	ATrackGenerator();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Initialize track generator */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void Initialize(ARabbitCharacter* InPlayerCharacter);

	/** Update player reference without resetting track (for respawn) */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void UpdatePlayerReference(ARabbitCharacter* InPlayerCharacter);

	/** Reset track generator (clears all pieces) */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void Reset();

	/** Get current difficulty level */
	UFUNCTION(BlueprintPure, Category = "Track")
	int32 GetCurrentDifficulty() const { return CurrentDifficulty; }

	/** Set current difficulty level */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetCurrentDifficulty(int32 NewDifficulty) { CurrentDifficulty = NewDifficulty; }

	/** Get distance traveled in meters (1 meter = 800 game units) */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetDistanceTraveled() const { return DistanceTraveled / 800.0f; }

	/** Get active track pieces (for ground detection) */
	UFUNCTION(BlueprintPure, Category = "Track")
	TArray<ATrackPiece*> GetActiveTrackPieces() const { return ActiveTrackPieces; }

	/** Get total track pieces spawned during this run */
	UFUNCTION(BlueprintPure, Category = "Track")
	int32 GetTotalTrackPiecesSpawned() const { return TotalTrackPiecesSpawned; }

	/** Get current piece sequence IDs */
	TArray<FString> GetCurrentPieceIds() const;

	/** Load a finite track sequence */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void LoadTrackSequence(const FTrackSequenceData& SequenceData);

	/** Get remaining pieces in sequence */
	UFUNCTION(BlueprintPure, Category = "Track")
	int32 GetRemainingPieces() const;

	/** Check if track sequence is loaded */
	UFUNCTION(BlueprintPure, Category = "Track")
	bool IsTrackSequenceLoaded() const { return bTrackSequenceLoaded; }

	/** Set endless mode */
	UFUNCTION(BlueprintCallable, Category = "Track")
	void SetEndlessMode(bool bEnabled) { bEndlessMode = bEnabled; }

	/** Delegates for shop/boss events */
	UPROPERTY(BlueprintAssignable, Category = "Track")
	FOnShopPieceReached OnShopPieceReached;

	UPROPERTY(BlueprintAssignable, Category = "Track")
	FOnBossPieceReached OnBossPieceReached;

private:
	/** Select a track piece definition based on difficulty and weight (legacy - for endless mode) */
	UTrackPieceDefinition* SelectTrackPieceDefinition();

	/** Find the first piece definition (type Start) */
	UTrackPieceDefinition* FindFirstPieceDefinition() const;

	/** Find track piece definition by content ID */
	UTrackPieceDefinition* FindTrackPieceDefinitionById(const FString& ContentId) const;

protected:
	/** Player character reference */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References")
	ARabbitCharacter* PlayerCharacter;

	/** Track piece definitions to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TArray<UTrackPieceDefinition*> TrackPieceDefinitions;

	/** Track piece class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TSubclassOf<ATrackPiece> TrackPieceClass;

	/** Number of track pieces to keep ahead of player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "1", ClampMax = "20"))
	int32 PiecesAhead = 5;

	/** Number of track pieces to keep behind player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "0", ClampMax = "10"))
	int32 PiecesBehind = 2;

	/** Distance to spawn pieces ahead */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
	float SpawnDistanceAhead = 5000.0f;

	/** Distance to destroy pieces behind */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
	float DestroyDistanceBehind = 2000.0f;

	/** Current difficulty level */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track")
	int32 CurrentDifficulty = 0;

	/** Distance traveled by player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track")
	float DistanceTraveled = 0.0f;

	/** Last spawn position */
	float LastSpawnPosition = 0.0f;

	/** Pool of active track pieces */
	UPROPERTY()
	TArray<ATrackPiece*> ActiveTrackPieces;

	/** Total track pieces spawned during this run */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track")
	int32 TotalTrackPiecesSpawned = 0;

	/** Timer for throttling spawn checks (don't check every frame) */
	float SpawnCheckTimer = 0.0f;
	
	/** Interval between spawn checks (seconds) */
	static constexpr float SpawnCheckInterval = 0.5f;

	/** Track sequence data (if using finite tracks) */
	UPROPERTY()
	FTrackSequenceData TrackSequenceData;

	/** Current piece index in sequence */
	int32 CurrentPieceIndex = 0;

	/** Whether track sequence is loaded */
	bool bTrackSequenceLoaded = false;

	/** Whether in endless mode (infinite generation) */
	bool bEndlessMode = false;

	/** Map of spawned pieces to their content IDs (for shop/boss detection) */
	TMap<ATrackPiece*, FString> PieceIdMap;

	/** Set of pieces that have already triggered events */
	TSet<ATrackPiece*> ReachedPieces;

	/** Spawn a new track piece (from sequence or random for endless) */
	void SpawnTrackPiece(float Position);

	/** Spawn next piece from sequence */
	void SpawnNextSequencePiece();

	/** Destroy track pieces that are too far behind */
	void CleanupOldPieces();

	/** Create track piece from definition */
	ATrackPiece* CreateTrackPieceFromDefinition(UTrackPieceDefinition* Definition, const FVector& ConnectionPoint);

	/** Draw debug visualization for lanes */
	void DrawLaneDebugVisualization();
};
