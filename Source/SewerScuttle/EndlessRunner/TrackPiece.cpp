// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrackPiece.h"
#include "ObstacleSpawnComponent.h"
#include "PowerUpSpawnComponent.h"
#include "CollectibleSpawnComponent.h"
#include "Obstacle.h"
#include "PowerUp.h"
#include "CollectibleCoin.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ATrackPiece::ATrackPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Create start connection point (where previous piece connects)
	StartConnection = CreateDefaultSubobject<USceneComponent>(TEXT("StartConnection"));
	StartConnection->SetupAttachment(RootSceneComponent);
	StartConnection->SetRelativeLocation(FVector::ZeroVector);
	StartConnection->SetComponentTickEnabled(false); // No need to tick
	StartConnection->PrimaryComponentTick.bCanEverTick = false;

	// Create default end connection point (where next piece connects)
	USceneComponent* DefaultEndConnection = CreateDefaultSubobject<USceneComponent>(TEXT("EndConnection"));
	DefaultEndConnection->SetupAttachment(RootSceneComponent);
	DefaultEndConnection->SetRelativeLocation(FVector(Length, 0.0f, 0.0f)); // Default to Length units forward
	DefaultEndConnection->SetComponentTickEnabled(false); // No need to tick
	DefaultEndConnection->PrimaryComponentTick.bCanEverTick = false;
	EndConnections.Add(DefaultEndConnection);
}

void ATrackPiece::BeginPlay()
{
	Super::BeginPlay();
	
	// Build component cache once for fast lookups
	BuildComponentCache();
}

void ATrackPiece::AddSpawnPoint(const FSpawnPoint& SpawnPoint)
{
	SpawnPoints.Add(SpawnPoint);
}

void ATrackPiece::ClearSpawnPoints()
{
	SpawnPoints.Empty();
}

void ATrackPiece::SetStartConnectionByName(const FString& ComponentName)
{
	if (ComponentName.IsEmpty())
	{
		// Use default if name is empty
		return;
	}
	
	USceneComponent* FoundComponent = FindComponentByName(ComponentName);
	if (FoundComponent)
	{
		StartConnection = FoundComponent;
		UE_LOG(LogTemp, Log, TEXT("TrackPiece: Set StartConnection to component '%s' (found)"), *ComponentName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPiece: StartConnection component '%s' not found, using default. Available components:"), *ComponentName);
		for (const auto& Pair : ComponentCache)
		{
			UE_LOG(LogTemp, Warning, TEXT("  - '%s'"), *Pair.Key);
		}
	}
}

void ATrackPiece::SetEndConnectionByName(const FString& ComponentName)
{
	if (ComponentName.IsEmpty())
	{
		// Use default if name is empty
		return;
	}
	
	USceneComponent* FoundComponent = FindComponentByName(ComponentName);
	if (FoundComponent)
	{
		EndConnections.Empty();
		EndConnections.Add(FoundComponent);
		UE_LOG(LogTemp, Log, TEXT("TrackPiece: Set EndConnection to component '%s'"), *ComponentName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPiece: EndConnection component '%s' not found, using default"), *ComponentName);
	}
}

void ATrackPiece::SetEndConnectionsByNames(const TArray<FString>& ComponentNames)
{
	if (ComponentNames.Num() == 0)
	{
		// Use default if array is empty
		return;
	}
	
	EndConnections.Empty();
	
	for (const FString& ComponentName : ComponentNames)
	{
		if (ComponentName.IsEmpty())
		{
			continue; // Skip empty names
		}
		
		USceneComponent* FoundComponent = FindComponentByName(ComponentName);
		if (FoundComponent)
		{
			EndConnections.Add(FoundComponent);
			UE_LOG(LogTemp, Log, TEXT("TrackPiece: Added EndConnection component '%s' (found)"), *ComponentName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackPiece: EndConnection component '%s' not found, skipping. Available components:"), *ComponentName);
			for (const auto& Pair : ComponentCache)
			{
				UE_LOG(LogTemp, Warning, TEXT("  - '%s'"), *Pair.Key);
			}
		}
	}
	
	if (EndConnections.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackPiece: No valid EndConnection components found, using default"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TrackPiece: Set %d EndConnection components"), EndConnections.Num());
	}
}

FVector ATrackPiece::GetStartConnectionWorldPosition() const
{
	if (StartConnection)
	{
		return StartConnection->GetComponentLocation();
	}
	// Fallback to actor location
	return GetActorLocation();
}

FVector ATrackPiece::GetEndConnectionWorldPosition() const
{
	if (EndConnections.Num() > 0 && EndConnections[0])
	{
		return EndConnections[0]->GetComponentLocation();
	}
	// Fallback: calculate from Length
	FVector StartPos = GetStartConnectionWorldPosition();
	return StartPos + GetActorForwardVector() * Length;
}

FVector ATrackPiece::GetEndConnectionWorldPositionByIndex(int32 Index) const
{
	if (EndConnections.IsValidIndex(Index) && EndConnections[Index])
	{
		return EndConnections[Index]->GetComponentLocation();
	}
	// Fallback to first end connection or calculated position
	return GetEndConnectionWorldPosition();
}

void ATrackPiece::BuildComponentCache()
{
	ComponentCache.Empty();
	
	// Search through all components once and cache them
	TArray<UActorComponent*> Components;
	GetComponents(USceneComponent::StaticClass(), Components);

	for (UActorComponent* Component : Components)
	{
		if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
		{
			FString CompName = SceneComp->GetName();
			ComponentCache.Add(CompName, SceneComp);
			
			// Auto-add spawn points from components if they are not already in the array
			bool bIsSpawnComponent = false;
			FSpawnPoint NewPoint;
			NewPoint.SpawnPositionComponentName = CompName;
			
			if (UObstacleSpawnComponent* ObsComp = Cast<UObstacleSpawnComponent>(SceneComp))
			{
				NewPoint.SpawnType = ESpawnPointType::Obstacle;
				NewPoint.WeightedDefinitions = ObsComp->ObstacleDefinitions;
				bIsSpawnComponent = true;
			}
			else if (UPowerUpSpawnComponent* PUComp = Cast<UPowerUpSpawnComponent>(SceneComp))
			{
				NewPoint.SpawnType = ESpawnPointType::PowerUp;
				NewPoint.WeightedDefinitions = PUComp->PowerUpDefinitions;
				bIsSpawnComponent = true;
			}
			else if (UCollectibleSpawnComponent* ColComp = Cast<UCollectibleSpawnComponent>(SceneComp))
			{
				NewPoint.SpawnType = ESpawnPointType::Coin;
				NewPoint.WeightedDefinitions = ColComp->CollectibleDefinitions;
				bIsSpawnComponent = true;
			}

			if (bIsSpawnComponent)
			{
				// Determine lane and forward position from relative location
				float Y = SceneComp->GetRelativeLocation().Y;
				NewPoint.Lane = 1; // Center
				if (Y < -100.0f) NewPoint.Lane = 0;
				else if (Y > 100.0f) NewPoint.Lane = 2;
				
				NewPoint.ForwardPosition = SceneComp->GetRelativeLocation().X;

				// Check if this component is already registered (to avoid duplicates)
				bool bAlreadyRegistered = false;
				for (const FSpawnPoint& ExistingPoint : SpawnPoints)
				{
					if (ExistingPoint.SpawnPositionComponentName == CompName)
					{
						bAlreadyRegistered = true;
						break;
					}
				}

				if (!bAlreadyRegistered)
				{
					SpawnPoints.Add(NewPoint);
					UE_LOG(LogTemp, Log, TEXT("TrackPiece: Auto-registered spawn point from component '%s'"), *CompName);
				}
			}

			// Disable ticking on all SceneComponents (they're just position markers)
			SceneComp->SetComponentTickEnabled(false);
			SceneComp->PrimaryComponentTick.bCanEverTick = false;
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("TrackPiece: Built component cache with %d components for '%s'"), ComponentCache.Num(), *GetName());
	
	// Log all component names for debugging
	for (const auto& Pair : ComponentCache)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("TrackPiece: Cached component '%s'"), *Pair.Key);
	}
}

USceneComponent* ATrackPiece::FindComponentByName(const FString& ComponentName) const
{
	if (ComponentName.IsEmpty())
	{
		return nullptr;
	}

	// Use cached lookup (O(1) instead of O(n))
	const USceneComponent* const* FoundComponent = ComponentCache.Find(ComponentName);
	if (FoundComponent)
	{
		// Safe to cast away const - we're not modifying the component, just returning it
		return const_cast<USceneComponent*>(*FoundComponent);
	}

	// Not found in cache (shouldn't happen if BuildComponentCache was called)
	UE_LOG(LogTemp, Warning, TEXT("TrackPiece: Component '%s' not found in cache for '%s'"), *ComponentName, *GetName());
	return nullptr;
}

void ATrackPiece::DrawLaneVisualization(float Duration, float Height)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Fixed lane coordinates (must match RabbitCharacter and SpawnManager)
	constexpr float LANE_LEFT_Y = -200.0f;
	constexpr float LANE_CENTER_Y = 0.0f;
	constexpr float LANE_RIGHT_Y = 200.0f;
	constexpr float LANE_WIDTH = 200.0f;
	constexpr float LANE_HALF_WIDTH = LANE_WIDTH * 0.5f;

	// Get track piece bounds
	FVector TrackLocation = GetActorLocation();
	FVector TrackForward = GetActorForwardVector();
	FVector TrackRight = GetActorRightVector();
	FVector TrackUp = GetActorUpVector();
	
	// Use Length for visualization distance
	float VisualLength = Length;
	if (VisualLength < 100.0f)
	{
		VisualLength = 1000.0f; // Default if Length not set
	}

	// Calculate start and end positions
	FVector StartPos = TrackLocation;
	FVector EndPos = TrackLocation + TrackForward * VisualLength;
	
	// Height offset (default to track piece Z)
	float ZOffset = Height;
	if (FMath::IsNearlyZero(ZOffset))
	{
		ZOffset = TrackLocation.Z;
	}

	// Draw left lane (red)
	FVector LeftLaneStart = StartPos + TrackRight * LANE_LEFT_Y + TrackUp * ZOffset;
	FVector LeftLaneEnd = EndPos + TrackRight * LANE_LEFT_Y + TrackUp * ZOffset;
	DrawDebugLine(World, LeftLaneStart, LeftLaneEnd, FColor::Red, false, Duration, 0, 5.0f);
	
	// Draw left lane box
	FVector LeftLaneCenter = (LeftLaneStart + LeftLaneEnd) * 0.5f;
	FVector LeftLaneSize(VisualLength * 0.5f, LANE_HALF_WIDTH, 10.0f);
	DrawDebugBox(World, LeftLaneCenter, LeftLaneSize, FColor::Red, false, Duration, 0, 2.0f);

	// Draw center lane (green)
	FVector CenterLaneStart = StartPos + TrackRight * LANE_CENTER_Y + TrackUp * ZOffset;
	FVector CenterLaneEnd = EndPos + TrackRight * LANE_CENTER_Y + TrackUp * ZOffset;
	DrawDebugLine(World, CenterLaneStart, CenterLaneEnd, FColor::Green, false, Duration, 0, 5.0f);
	
	// Draw center lane box
	FVector CenterLaneCenter = (CenterLaneStart + CenterLaneEnd) * 0.5f;
	FVector CenterLaneSize(VisualLength * 0.5f, LANE_HALF_WIDTH, 10.0f);
	DrawDebugBox(World, CenterLaneCenter, CenterLaneSize, FColor::Green, false, Duration, 0, 2.0f);

	// Draw right lane (blue)
	FVector RightLaneStart = StartPos + TrackRight * LANE_RIGHT_Y + TrackUp * ZOffset;
	FVector RightLaneEnd = EndPos + TrackRight * LANE_RIGHT_Y + TrackUp * ZOffset;
	DrawDebugLine(World, RightLaneStart, RightLaneEnd, FColor::Blue, false, Duration, 0, 5.0f);
	
	// Draw right lane box
	FVector RightLaneCenter = (RightLaneStart + RightLaneEnd) * 0.5f;
	FVector RightLaneSize(VisualLength * 0.5f, LANE_HALF_WIDTH, 10.0f);
	DrawDebugBox(World, RightLaneCenter, RightLaneSize, FColor::Blue, false, Duration, 0, 2.0f);

	// Draw lane markers at start
	DrawDebugSphere(World, LeftLaneStart, 20.0f, 8, FColor::Red, false, Duration, 0, 3.0f);
	DrawDebugSphere(World, CenterLaneStart, 20.0f, 8, FColor::Green, false, Duration, 0, 3.0f);
	DrawDebugSphere(World, RightLaneStart, 20.0f, 8, FColor::Blue, false, Duration, 0, 3.0f);
}

void ATrackPiece::RegisterSpawnedActor(AActor* SpawnedActor)
{
	if (SpawnedActor && !SpawnedActors.Contains(SpawnedActor))
	{
		SpawnedActors.Add(SpawnedActor);
		UE_LOG(LogTemp, VeryVerbose, TEXT("TrackPiece: Registered spawned actor '%s' to track piece '%s' (total: %d)"),
			*SpawnedActor->GetName(), *GetName(), SpawnedActors.Num());
	}
}

void ATrackPiece::ClearSpawnedActors()
{
	UE_LOG(LogTemp, Log, TEXT("TrackPiece: Clearing %d spawned actors from track piece '%s'"), SpawnedActors.Num(), *GetName());
	
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	
	SpawnedActors.Empty();
}

void ATrackPiece::BeginDestroy()
{
	// Clear all spawned actors before destroying the track piece
	ClearSpawnedActors();
	
	Super::BeginDestroy();
}

