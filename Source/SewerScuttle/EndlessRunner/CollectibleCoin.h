// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "CollectibleCoin.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UParticleSystemComponent;

/**
 * Collectible coin actor
 * Automatically collected when player overlaps
 */
UCLASS()
class SEWERSCUTTLE_API ACollectibleCoin : public AActor
{
	GENERATED_BODY()
	
public:
	ACollectibleCoin();

	virtual void BeginPlay() override;

	/** Get coin value */
	UFUNCTION(BlueprintPure, Category = "!Coin")
	int32 GetValue() const { return Value; }

	/** Set coin value */
	UFUNCTION(BlueprintCallable, Category = "!Coin")
	void SetValue(int32 NewValue) { Value = NewValue; }

	/** Collect this coin */
	UFUNCTION(BlueprintCallable, Category = "!Coin")
	void Collect();

	/** Check if coin has been collected */
	UFUNCTION(BlueprintPure, Category = "!Coin")
	bool IsCollected() const { return bCollected; }

	/** Check if coin is magnetable */
	UFUNCTION(BlueprintPure, Category = "!Coin")
	bool IsMagnetable() const { return bMagnetable; }

	/** Get collision sphere component (for magnet system) */
	USphereComponent* GetCollisionSphere() const { return CollisionSphere; }

protected:
	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* CollisionSphere;

	/** Static mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Particle effect on collection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UParticleSystemComponent* CollectionEffect;

	/** Coin value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin", meta = (ClampMin = "1"))
	int32 Value = 1;

	/** Whether this coin can be attracted by magnet power-up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin")
	bool bMagnetable = true;

public:
	/** Whether this is a special collectible (for Collector class) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin")
	bool bIsSpecial = false;

	/** Value multiplier for special collectibles (default 2.0x) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin", meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float SpecialValueMultiplier = 2.0f;

protected:

	/** Rotation speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float RotationSpeed = 90.0f;

	/** Bobbing speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float BobSpeed = 2.0f;

	/** Bobbing amplitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Coin", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float BobAmplitude = 20.0f;

	/** Whether coin has been collected */
	bool bCollected = false;

	/** Initial Z position */
	float InitialZ = 0.0f;

	/** Handle overlap */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Tick(float DeltaTime) override;
};

