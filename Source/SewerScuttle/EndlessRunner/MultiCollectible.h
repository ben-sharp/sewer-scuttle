// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MultiCollectible.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;

/**
 * Structure defining a single collectible item within a multi-collectible actor
 */
USTRUCT(BlueprintType)
struct FCollectibleItem
{
	GENERATED_BODY()

	/** Static mesh component for this item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Collectible")
	UStaticMeshComponent* MeshComponent = nullptr;

	/** Collision sphere for this item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Collectible")
	USphereComponent* CollisionSphere = nullptr;

	/** Value of this collectible (coins, points, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible", meta = (ClampMin = "1"))
	int32 Value = 1;

	/** Whether this item has been collected */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Collectible")
	bool bCollected = false;

	/** Initial Z position for bobbing animation */
	float InitialZ = 0.0f;

	/** Initial rotation for rotation animation */
	FRotator InitialRotation = FRotator::ZeroRotator;
};

/**
 * Multi-collectible actor that can contain multiple collectible items (e.g., a trail of coins)
 * More performant than spawning multiple separate collectible actors
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API AMultiCollectible : public AActor
{
	GENERATED_BODY()
	
public:
	AMultiCollectible();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Get total value of all uncollected items */
	UFUNCTION(BlueprintPure, Category = "!Collectible")
	int32 GetTotalValue() const;

	/** Get number of uncollected items */
	UFUNCTION(BlueprintPure, Category = "!Collectible")
	int32 GetUncollectedCount() const;

	/** Collect all remaining items */
	UFUNCTION(BlueprintCallable, Category = "!Collectible")
	void CollectAll();

	/** Check if multi-collectible is magnetable */
	UFUNCTION(BlueprintPure, Category = "!Collectible")
	bool IsMagnetable() const { return bMagnetable; }

	/** Get collectible items array (for magnet system to access individual items) */
	const TArray<FCollectibleItem>& GetCollectibleItems() const { return CollectibleItems; }

	/** Collect a specific item by index (for magnet system) */
	UFUNCTION(BlueprintCallable, Category = "!Collectible")
	void CollectItem(int32 ItemIndex);

protected:
	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** Array of collectible items (populated in blueprint or via SetupItems) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "!Collectible")
	TArray<FCollectibleItem> CollectibleItems;

	/** Values for each collectible item (indexed by item index, defaults to 1 if not set) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible", meta = (ToolTip = "Set value for each item by index (0, 1, 2, etc.). Leave empty to use default value of 1."))
	TArray<int32> ItemValues;

	/** Default value for items if not specified in ItemValues array */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible", meta = (ClampMin = "1"))
	int32 DefaultItemValue = 1;

	/** Whether items in this multi-collectible can be attracted by magnet power-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible")
	bool bMagnetable = true;

public:
	/** Whether this is a special multi-collectible (for Collector class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible")
	bool bIsSpecial = false;

	/** Value multiplier for special collectibles (default 2.0x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Collectible", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float SpecialValueMultiplier = 2.0f;

protected:

	/** Rotation speed for items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float RotationSpeed = 90.0f;

	/** Bobbing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BobSpeed = 2.0f;

	/** Bobbing amplitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BobAmplitude = 20.0f;

	/** Whether to animate items */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bAnimateItems = true;

	/** Particle effect to play when an item is collected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* CollectionEffect = nullptr;

	/** Setup collectible items from child components (call this in blueprint or C++) */
	UFUNCTION(BlueprintCallable, Category = "!Collectible")
	void SetupItems();

	/** Handle overlap for a specific item */
	UFUNCTION()
	void OnItemOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/** Map of collision components to item indices for overlap handling */
	TMap<USphereComponent*, int32> CollisionToItemMap;
};

