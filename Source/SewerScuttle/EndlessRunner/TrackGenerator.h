// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackGenerator.generated.h"

class ATrackPiece;
class UTrackPieceDefinition;
class ARabbitCharacter;

/**
 * Manages endless track generation
 * Spawns track pieces ahead of player and destroys pieces behind
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

private:
	/** Select a track piece definition based on difficulty and weight */
	UTrackPieceDefinition* SelectTrackPieceDefinition();

	/** Find the first piece definition (marked with bIsFirstPiece) */
	UTrackPieceDefinition* FindFirstPieceDefinition() const;

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

	/** Spawn a new track piece */
	void SpawnTrackPiece(float Position);

	/** Destroy track pieces that are too far behind */
	void CleanupOldPieces();

	/** Create track piece from definition */
	ATrackPiece* CreateTrackPieceFromDefinition(UTrackPieceDefinition* Definition, const FVector& ConnectionPoint);

	/** Draw debug visualization for lanes */
	void DrawLaneDebugVisualization();
};

