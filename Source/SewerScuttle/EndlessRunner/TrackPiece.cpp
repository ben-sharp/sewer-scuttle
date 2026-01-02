// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrackPiece.h"
#include "Obstacle.h"
#include "PowerUp.h"
#include "CollectibleCoin.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

USmartSpawnComponent::USmartSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

ATrackPiece::ATrackPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root scene component
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// Create start connection point
	StartConnection = CreateDefaultSubobject<USceneComponent>(TEXT("StartConnection"));
	StartConnection->SetupAttachment(RootSceneComponent);
	StartConnection->SetRelativeLocation(FVector::ZeroVector);
	StartConnection->PrimaryComponentTick.bCanEverTick = false;

	// Create default end connection point
	USceneComponent* DefaultEndConnection = CreateDefaultSubobject<USceneComponent>(TEXT("EndConnection"));
	DefaultEndConnection->SetupAttachment(RootSceneComponent);
	DefaultEndConnection->SetRelativeLocation(FVector(Length, 0.0f, 0.0f));
	DefaultEndConnection->PrimaryComponentTick.bCanEverTick = false;
	EndConnections.Add(DefaultEndConnection);
}

void ATrackPiece::BeginPlay()
{
	Super::BeginPlay();
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
	if (ComponentName.IsEmpty()) return;
	USceneComponent* FoundComponent = FindComponentByName(ComponentName);
	if (FoundComponent) StartConnection = FoundComponent;
}

void ATrackPiece::SetEndConnectionByName(const FString& ComponentName)
{
	if (ComponentName.IsEmpty()) return;
	USceneComponent* FoundComponent = FindComponentByName(ComponentName);
	if (FoundComponent)
	{
		EndConnections.Empty();
		EndConnections.Add(FoundComponent);
	}
}

void ATrackPiece::SetEndConnectionsByNames(const TArray<FString>& ComponentNames)
{
	if (ComponentNames.Num() == 0) return;
	EndConnections.Empty();
	for (const FString& ComponentName : ComponentNames)
	{
		USceneComponent* FoundComponent = FindComponentByName(ComponentName);
		if (FoundComponent) EndConnections.Add(FoundComponent);
	}
}

FVector ATrackPiece::GetStartConnectionWorldPosition() const
{
	return StartConnection ? StartConnection->GetComponentLocation() : GetActorLocation();
}

FVector ATrackPiece::GetEndConnectionWorldPosition() const
{
	if (EndConnections.Num() > 0 && EndConnections[0]) return EndConnections[0]->GetComponentLocation();
	return GetStartConnectionWorldPosition() + GetActorForwardVector() * Length;
}

FVector ATrackPiece::GetEndConnectionWorldPositionByIndex(int32 Index) const
{
	if (EndConnections.IsValidIndex(Index) && EndConnections[Index]) return EndConnections[Index]->GetComponentLocation();
	return GetEndConnectionWorldPosition();
}

void ATrackPiece::BuildComponentCache()
{
	ComponentCache.Empty();
	TArray<USceneComponent*> Components;
	GetComponents(USceneComponent::StaticClass(), Components);

	for (USceneComponent* SceneComp : Components)
	{
		FString CompName = SceneComp->GetName();
		ComponentCache.Add(CompName, SceneComp);

		if (USmartSpawnComponent* SmartComp = Cast<USmartSpawnComponent>(SceneComp))
		{
			FSpawnPoint NewPoint;
			NewPoint.SpawnPositionComponentName = CompName;
			NewPoint.WeightedDefinitions = SmartComp->Definitions;
			NewPoint.SpawnProbability = SmartComp->SpawnProbability;
			
			// Auto-calculate relative lane
			float Y = SceneComp->GetRelativeLocation().Y;
			NewPoint.Lane = (Y < -100.0f) ? 0 : ((Y > 100.0f) ? 2 : 1);
			NewPoint.ForwardPosition = SceneComp->GetRelativeLocation().X;

			// Add if unique
			bool bAlreadyRegistered = false;
			for (const FSpawnPoint& P : SpawnPoints)
			{
				if (P.SpawnPositionComponentName == CompName) { bAlreadyRegistered = true; break; }
			}
			if (!bAlreadyRegistered) SpawnPoints.Add(NewPoint);
		}

		SceneComp->SetComponentTickEnabled(false);
	}
}

USceneComponent* ATrackPiece::FindComponentByName(const FString& ComponentName) const
{
	const USceneComponent* const* FoundComponent = ComponentCache.Find(ComponentName);
	return FoundComponent ? const_cast<USceneComponent*>(*FoundComponent) : nullptr;
}

void ATrackPiece::DrawLaneVisualization(float Duration, float Height)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector TrackLocation = GetActorLocation();
	FVector TrackForward = GetActorForwardVector();
	FVector TrackRight = GetActorRightVector();
	
	FVector StartPos = TrackLocation;
	FVector EndPos = TrackLocation + TrackForward * Length;
	float Z = (Height != 0.0f) ? Height : TrackLocation.Z;

	float Lanes[] = { -200.0f, 0.0f, 200.0f };
	FColor Colors[] = { FColor::Red, FColor::Green, FColor::Blue };

	for (int i = 0; i < 3; i++)
	{
		FVector LStart = StartPos + TrackRight * Lanes[i] + FVector(0,0,Z);
		FVector LEnd = EndPos + TrackRight * Lanes[i] + FVector(0,0,Z);
		DrawDebugLine(World, LStart, LEnd, Colors[i], false, Duration, 0, 5.0f);
	}
}

void ATrackPiece::RegisterSpawnedActor(AActor* SpawnedActor)
{
	if (SpawnedActor) SpawnedActors.AddUnique(SpawnedActor);
}

void ATrackPiece::ClearSpawnedActors()
{
	for (AActor* Actor : SpawnedActors) if (IsValid(Actor)) Actor->Destroy();
	SpawnedActors.Empty();
}

void ATrackPiece::BeginDestroy()
{
	ClearSpawnedActors();
	Super::BeginDestroy();
}
