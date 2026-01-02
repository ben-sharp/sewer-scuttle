// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Obstacle.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class ARabbitCharacter;

UENUM(BlueprintType)
enum class EObstacleType : uint8
{
	Low		UMETA(DisplayName = "Low - Slide Under"),
	High	UMETA(DisplayName = "High - Jump Over"),
	Full	UMETA(DisplayName = "Full - Must Avoid")
};

namespace ObstacleUtils
{
	static FString ToString(EObstacleType Type)
	{
		switch (Type)
		{
			case EObstacleType::Low: return TEXT("Low");
			case EObstacleType::High: return TEXT("High");
			case EObstacleType::Full:
			default: return TEXT("Full");
		}
	}

	static EObstacleType FromString(const FString& TypeStr)
	{
		if (TypeStr.Equals(TEXT("Low"), ESearchCase::IgnoreCase)) return EObstacleType::Low;
		if (TypeStr.Equals(TEXT("High"), ESearchCase::IgnoreCase)) return EObstacleType::High;
		return EObstacleType::Full;
	}
}

/**
 * Base obstacle actor
 * Blocks player path and causes damage/knockback on collision
 */
UCLASS()
class SEWERSCUTTLE_API AObstacle : public AActor
{
	GENERATED_BODY()
	
public:
	AObstacle();

	virtual void BeginPlay() override;

	/** Get obstacle type */
	UFUNCTION(BlueprintPure, Category = "!Obstacle")
	EObstacleType GetObstacleType() const { return ObstacleType; }

	/** Set obstacle type */
	UFUNCTION(BlueprintCallable, Category = "!Obstacle")
	void SetObstacleType(EObstacleType NewType) { ObstacleType = NewType; }

	/** Get damage amount */
	UFUNCTION(BlueprintPure, Category = "!Obstacle")
	float GetDamage() const { return Damage; }

	/** Set damage amount */
	UFUNCTION(BlueprintCallable, Category = "!Obstacle")
	void SetDamage(float NewDamage) { Damage = NewDamage; }

protected:
	/** Collision box */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* CollisionBox;

	/** Static mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	/** Obstacle type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle")
	EObstacleType ObstacleType = EObstacleType::Full;

	/** Damage dealt on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Damage = 10.0f;

	/** Knockback force on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle", meta = (ClampMin = "0.0", ClampMax = "5000.0"))
	float KnockbackForce = 1000.0f;

	/** Lives lost on collision (default 1, can be set higher for deadly obstacles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle", meta = (ClampMin = "1", ClampMax = "10"))
	int32 LivesLost = 1;

	/** Whether this obstacle can be broken by Enforcer class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle")
	bool bIsBreakable = false;

	/** Particle effect to play when obstacle is broken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle")
	class UParticleSystem* BreakEffect = nullptr;

	/** If true, this obstacle bypasses last stand and instantly kills if LivesLost >= player's current lives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "!Obstacle", meta = (ToolTip = "If true and LivesLost >= player's current lives, instantly kills player (bypasses last stand)"))
	bool bIgnoreLastStand = false;

	/** Handle overlap */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Handle collision with player */
	UFUNCTION(BlueprintImplementableEvent, Category = "!Obstacle")
	void OnPlayerCollision(ARabbitCharacter* Player);
};

