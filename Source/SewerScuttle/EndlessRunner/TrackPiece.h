// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackPiece.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UENUM(BlueprintType)
enum class ESpawnPointType : uint8
{
	Coin		UMETA(DisplayName = "Coin"),
	PowerUp		UMETA(DisplayName = "PowerUp"),
	Obstacle	UMETA(DisplayName = "Obstacle")
};

USTRUCT(BlueprintType)
struct FSpawnPoint
{
	GENERATED_BODY()

	/** Lane position (0 = Left, 1 = Center, 2 = Right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Lane = 1;

	/** Position along track (forward distance) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ForwardPosition = 0.0f;

	/** Type of spawn point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESpawnPointType SpawnType = ESpawnPointType::Coin;

	/** Class to spawn at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpawnClass;

	/** Optional: Name of a Scene component in the track piece to use as spawn position (overrides Lane/ForwardPosition) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpawnPositionComponentName;
};

/**
 * Track piece actor for endless runner
 * Contains spawn points for collectibles and obstacles
 */
UCLASS()
class SEWERSCUTTLE_API ATrackPiece : public AActor
{
	GENERATED_BODY()
	
public:
	ATrackPiece();

	virtual void BeginPlay() override;

	/** Get length of this track piece */
	UFUNCTION(BlueprintPure, Category = "!Track")
	float GetLength() const { return Length; }

	/** Set length of this track piece */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void SetLength(float NewLength) { Length = NewLength; }

	/** Get spawn points */
	UFUNCTION(BlueprintPure, Category = "!Track")
	const TArray<FSpawnPoint>& GetSpawnPoints() const { return SpawnPoints; }

	/** Add spawn point */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void AddSpawnPoint(const FSpawnPoint& SpawnPoint);

	/** Clear all spawn points */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void ClearSpawnPoints();

	/** Get root scene component */
	UFUNCTION(BlueprintPure, Category = "!Track")
	USceneComponent* GetRootSceneComponent() const { return RootSceneComponent; }

	/** Get start connection point (where previous piece connects) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	USceneComponent* GetStartConnection() const { return StartConnection; }

	/** Set start connection by component name (for blueprints) */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void SetStartConnectionByName(const FString& ComponentName);

	/** Get end connection points (where next pieces can connect - supports multiple for turns) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	const TArray<USceneComponent*>& GetEndConnections() const { return EndConnections; }

	/** Set end connection by component name (for blueprints) - clears existing and adds this one */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void SetEndConnectionByName(const FString& ComponentName);

	/** Set end connections by component names (for blueprints) - clears existing and adds all specified */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void SetEndConnectionsByNames(const TArray<FString>& ComponentNames);

	/** Get world position of start connection point */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetStartConnectionWorldPosition() const;

	/** Get world position of first end connection point (primary forward connection) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetEndConnectionWorldPosition() const;

	/** Get world position of end connection point by index */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetEndConnectionWorldPositionByIndex(int32 Index) const;

	/** Find a Scene component by name (uses cached lookup for performance) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	USceneComponent* FindComponentByName(const FString& ComponentName) const;

	/** Build component cache (called once in BeginPlay, but can be called manually) */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void BuildComponentCache();

	/** Draw lane visualization (for editor/runtime debugging) */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "!Track|Debug")
	void DrawLaneVisualization(float Duration = 0.0f, float Height = 0.0f);

	/** Register a spawned actor (coin, obstacle, powerup, etc.) to be destroyed with this track piece */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void RegisterSpawnedActor(AActor* SpawnedActor);

	/** Clear all spawned actors (called when track piece is destroyed) */
	void ClearSpawnedActors();

protected:
	virtual void BeginDestroy() override;
	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** Start connection point (where previous piece connects to this one) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Track|Connection Points")
	USceneComponent* StartConnection;

	/** End connection points (where next pieces connect - supports multiple for turns) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Track|Connection Points")
	TArray<USceneComponent*> EndConnections;

	/** Length of this track piece in units (fallback if connection points not set) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Track", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
	float Length = 1000.0f;

	/** Spawn points for collectibles and obstacles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Track")
	TArray<FSpawnPoint> SpawnPoints;

	/** Lane width (should match player's lane width) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Track", meta = (ClampMin = "100.0", ClampMax = "500.0"))
	float LaneWidth = 200.0f;

	/** Actors spawned on this track piece (coins, obstacles, powerups, etc.) */
	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	/** Cached component lookup map (name -> component) for fast access */
	UPROPERTY()
	TMap<FString, USceneComponent*> ComponentCache;
};

