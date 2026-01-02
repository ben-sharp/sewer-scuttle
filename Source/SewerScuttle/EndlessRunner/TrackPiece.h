// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerClass.h"
#include "BaseContentDefinition.h"
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

namespace SpawnPointUtils
{
	static FString ToString(ESpawnPointType Type)
	{
		switch (Type)
		{
			case ESpawnPointType::PowerUp: return TEXT("PowerUp");
			case ESpawnPointType::Obstacle: return TEXT("Obstacle");
			case ESpawnPointType::Coin:
			default: return TEXT("Coin");
		}
	}

	static ESpawnPointType FromString(const FString& TypeStr)
	{
		if (TypeStr.Equals(TEXT("PowerUp"), ESearchCase::IgnoreCase)) return ESpawnPointType::PowerUp;
		if (TypeStr.Equals(TEXT("Obstacle"), ESearchCase::IgnoreCase)) return ESpawnPointType::Obstacle;
		return ESpawnPointType::Coin;
	}
}

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

	/** Possible definitions that can spawn here with weights (from components) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWeightedDefinition> WeightedDefinitions;

	/** Probability of ANY content spawning at this point (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnProbability = 1.0f;

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

	/** Set length of this track piece (called by Generator from DA) */
	void SetLength(float NewLength) { Length = NewLength; }

	/** Get lane width */
	UFUNCTION(BlueprintPure, Category = "!Track")
	float GetLaneWidth() const { return LaneWidth; }

	/** Set lane width (called by Generator from DA) */
	void SetLaneWidth(float NewWidth) { LaneWidth = NewWidth; }

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

	/** Set prescribed spawns from server */
	void SetPrescribedSpawns(const TMap<FString, FString>& Spawns) { PrescribedSpawns = Spawns; bHasPrescribedSpawns = true; }

	/** Get prescribed spawn for a component */
	FString GetPrescribedSpawn(const FString& ComponentName) const { const FString* S = PrescribedSpawns.Find(ComponentName); return S ? *S : TEXT(""); }

	/** Check if this piece has server-prescribed spawns */
	bool HasPrescribedSpawns() const { return bHasPrescribedSpawns; }

	/** Get world position of start connection point */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetStartConnectionWorldPosition() const;

	/** Get world position of first end connection point (primary forward connection) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetEndConnectionWorldPosition() const;

	/** Get world position of end connection point by index */
	UFUNCTION(BlueprintPure, Category = "!Track")
	FVector GetEndConnectionWorldPositionByIndex(int32 Index) const;

	/** Register a spawned actor (coin, obstacle, powerup, etc.) to be destroyed with this track piece */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void RegisterSpawnedActor(AActor* SpawnedActor);

	/** Clear all spawned actors (called when track piece is destroyed) */
	void ClearSpawnedActors();

	/** Find a Scene component by name (uses cached lookup for performance) */
	UFUNCTION(BlueprintPure, Category = "!Track")
	USceneComponent* FindComponentByName(const FString& ComponentName) const;

	/** Build component cache (called once in BeginPlay, but can be called manually) */
	UFUNCTION(BlueprintCallable, Category = "!Track")
	void BuildComponentCache();

	/** Draw lane visualization (for editor/runtime debugging) */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "!Track|Debug")
	void DrawLaneVisualization(float Duration = 0.0f, float Height = 0.0f);

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

	/** Length of this track piece in units (Applied from DA) */
	UPROPERTY(VisibleInstanceOnly, Category = "!Track")
	float Length = 1000.0f;

	/** Spawn points for collectibles and obstacles (Populated from Components) */
	UPROPERTY(VisibleInstanceOnly, Category = "!Track")
	TArray<FSpawnPoint> SpawnPoints;

	/** Lane width (Applied from DA) */
	UPROPERTY(VisibleInstanceOnly, Category = "!Track")
	float LaneWidth = 200.0f;

	/** Actors spawned on this track piece (coins, obstacles, powerups, etc.) */
	UPROPERTY()
	TArray<AActor*> SpawnedActors;

	/** Prescribed spawns from server (ComponentName -> DefID) */
	UPROPERTY()
	TMap<FString, FString> PrescribedSpawns;

	/** Flag indicating if this piece has prescribed spawns from the server */
	UPROPERTY()
	bool bHasPrescribedSpawns = false;

	/** Cached component lookup map (name -> component) for fast access */
	UPROPERTY()
	TMap<FString, USceneComponent*> ComponentCache;
};

/**
 * Unified smart spawn component for any content type
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SEWERSCUTTLE_API USmartSpawnComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USmartSpawnComponent();

	/** Weighted definitions that can spawn at this point (can mix Obstacles, PowerUps, and Coins) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FWeightedDefinition> Definitions;

	/** Probability of ANY content spawning at this point (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 1.0f;
};
