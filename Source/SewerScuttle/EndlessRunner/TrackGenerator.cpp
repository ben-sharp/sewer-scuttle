// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrackGenerator.h"
#include "TrackPiece.h"
#include "TrackPieceDefinition.h"
#include "RabbitCharacter.h"
#include "EndlessRunnerGameMode.h"
#include "GameplayManager.h"
#include "SpawnManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "DrawDebugHelpers.h"

ATrackGenerator::ATrackGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerCharacter = nullptr;
	CurrentDifficulty = 0;
	DistanceTraveled = 0.0f;
	LastSpawnPosition = 0.0f;
}

void ATrackGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void ATrackGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerCharacter)
	{
		return;
	}

	// Update distance traveled
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	DistanceTraveled = PlayerLocation.X;

	// Draw debug lane visualization (only in editor or with debug flag)
#if WITH_EDITOR
	// Only draw in editor, not in shipping builds
	// DrawLaneDebugVisualization(); // Disabled for performance
#endif

	// Throttle spawn checks - don't check every frame since pieces spawn far ahead
	SpawnCheckTimer += DeltaTime;
	if (SpawnCheckTimer >= SpawnCheckInterval)
	{
		SpawnCheckTimer = 0.0f;
		
		// Spawn new pieces if needed (limit to 1 per check for performance)
		float DistanceAhead = DistanceTraveled + SpawnDistanceAhead;
		if (LastSpawnPosition < DistanceAhead)
		{
			// Only spawn 1 piece per check to avoid lag spikes
			SpawnTrackPiece(LastSpawnPosition);
		}
	}

	// Cleanup old pieces (also throttle this)
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 1.0f) // Check cleanup every second
	{
		CleanupTimer = 0.0f;
		CleanupOldPieces();
	}
}

void ATrackGenerator::Initialize(ARabbitCharacter* InPlayerCharacter)
{
	PlayerCharacter = InPlayerCharacter;

	// Debug logging
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator::Initialize - TrackPieceDefinitions count: %d"), TrackPieceDefinitions.Num());
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator::Initialize - TrackPieceClass: %s"), TrackPieceClass ? *TrackPieceClass->GetName() : TEXT("NULL"));

	if (TrackPieceDefinitions.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: TrackPieceDefinitions array is EMPTY! Add data assets in blueprint."));
		return;
	}

	if (!TrackPieceClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: TrackPieceClass is NULL! Set BP_TrackPiece in blueprint."));
		return;
	}

	// Spawn initial track pieces
	// Start at position 0 for the first piece
	FVector FirstConnectionPoint(0.0f, 0.0f, 0.0f);
	LastSpawnPosition = 0.0f;
	
	// Try to find and use the first piece definition
	UTrackPieceDefinition* FirstPieceDefinition = FindFirstPieceDefinition();
	
	for (int32 i = 0; i < PiecesAhead; ++i)
	{
		// For the very first piece (i == 0), use the first piece definition if available
		FVector ConnectionPoint = (i == 0) ? FirstConnectionPoint : FVector(LastSpawnPosition, 0.0f, 0.0f);
		
		if (i == 0 && FirstPieceDefinition)
		{
			// Spawn the designated first piece
			ATrackPiece* NewPiece = CreateTrackPieceFromDefinition(FirstPieceDefinition, ConnectionPoint);
			if (NewPiece)
			{
				ActiveTrackPieces.Add(NewPiece);
				// Get the end connection position from the first piece
				FVector EndConnectionPos = NewPiece->GetEndConnectionWorldPosition();
				LastSpawnPosition = EndConnectionPos.X;
				
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Spawned first piece '%s' at start, end connection at X=%.2f"), 
					*FirstPieceDefinition->PieceName, LastSpawnPosition);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to spawn first piece, falling back to normal selection"));
				SpawnTrackPiece(ConnectionPoint.X);
			}
		}
		else
		{
			// Use normal selection for subsequent pieces
			// Make sure we're using the correct connection point from the last piece
			if (ActiveTrackPieces.Num() > 0)
			{
				ATrackPiece* LastPiece = ActiveTrackPieces.Last();
				if (LastPiece)
				{
					FVector EndConnectionPos = LastPiece->GetEndConnectionWorldPosition();
					ConnectionPoint = FVector(EndConnectionPos.X, 0.0f, 0.0f);
					LastSpawnPosition = EndConnectionPos.X;
					
					// Safety check: ensure we're not spawning inside the first piece
					if (i == 1 && ConnectionPoint.X < 100.0f)
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Warning - second piece would spawn at X=%.2f (too close to start). First piece end connection may be incorrect."), 
							ConnectionPoint.X);
					}
					
					UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Initializing piece %d, connecting to previous piece end at X=%.2f"), 
						i, ConnectionPoint.X);
				}
			}
			// SpawnTrackPiece will use the connection point from the last piece
			SpawnTrackPiece(LastSpawnPosition);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator::Initialize - Spawned %d initial pieces"), ActiveTrackPieces.Num());
}

void ATrackGenerator::UpdatePlayerReference(ARabbitCharacter* InPlayerCharacter)
{
	PlayerCharacter = InPlayerCharacter;
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Updated player reference (track continues)"));
}

void ATrackGenerator::Reset()
{
	// Clear all existing track pieces and their spawned actors
	for (ATrackPiece* Piece : ActiveTrackPieces)
	{
		if (IsValid(Piece))
		{
			// Clear spawned actors first (coins, obstacles, power-ups, etc.)
			Piece->ClearSpawnedActors();
			// Then destroy the track piece itself
			Piece->Destroy();
		}
	}
	ActiveTrackPieces.Empty();
	
	// Reset tracking variables
	LastSpawnPosition = 0.0f;
	DistanceTraveled = 0.0f;
	PlayerCharacter = nullptr;
	
	UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Reset - cleared all track pieces and spawned actors"));
}

void ATrackGenerator::SpawnTrackPiece(float Position)
{
	UTrackPieceDefinition* Definition = SelectTrackPieceDefinition();
	if (!Definition)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: No valid track piece definition found"));
		return;
	}

	// Get the connection point from the previous piece (if any)
	FVector ConnectionPoint(Position, 0.0f, 0.0f);
	if (ActiveTrackPieces.Num() > 0)
	{
		ATrackPiece* LastPiece = ActiveTrackPieces.Last();
		if (LastPiece)
		{
			// Only use X coordinate from end connection for forward progression
			// Ignore Y/Z offsets (they might be wrong in blueprint)
			FVector EndConnectionPos = LastPiece->GetEndConnectionWorldPosition();
			ConnectionPoint = FVector(EndConnectionPos.X, 0.0f, 0.0f);
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Connecting to previous piece end at X=%.2f (ignoring Y/Z offsets)"), 
				ConnectionPoint.X);
		}
	}

	ATrackPiece* NewPiece = CreateTrackPieceFromDefinition(Definition, ConnectionPoint);
	if (NewPiece)
	{
		ActiveTrackPieces.Add(NewPiece);
		// Update last spawn position to the end connection of the new piece
		LastSpawnPosition = NewPiece->GetEndConnectionWorldPosition().X;
		UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Spawned piece, next connection at X=%.2f"), LastSpawnPosition);
	}
}

void ATrackGenerator::CleanupOldPieces()
{
	float DestroyPosition = DistanceTraveled - DestroyDistanceBehind;

	for (int32 i = ActiveTrackPieces.Num() - 1; i >= 0; --i)
	{
		ATrackPiece* Piece = ActiveTrackPieces[i];
		if (Piece)
		{
			float PiecePosition = Piece->GetActorLocation().X;
			if (PiecePosition < DestroyPosition)
			{
				// Clear spawned actors first, then destroy the piece
				Piece->ClearSpawnedActors();
				Piece->Destroy();
				ActiveTrackPieces.RemoveAt(i);
			}
		}
	}
}

UTrackPieceDefinition* ATrackGenerator::FindFirstPieceDefinition() const
{
	// Find the piece marked as the first piece
	for (UTrackPieceDefinition* Definition : TrackPieceDefinitions)
	{
		if (Definition && Definition->bIsFirstPiece)
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Found first piece definition '%s'"), *Definition->PieceName);
			return Definition;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: No first piece definition found, will use random selection"));
	return nullptr;
}

UTrackPieceDefinition* ATrackGenerator::SelectTrackPieceDefinition()
{
	// Filter definitions by difficulty (exclude first pieces from normal selection)
	TArray<UTrackPieceDefinition*> ValidDefinitions;
	int32 TotalWeight = 0;

	for (UTrackPieceDefinition* Definition : TrackPieceDefinitions)
	{
		if (Definition)
		{
			// Skip first pieces in normal selection
			if (Definition->bIsFirstPiece)
			{
				continue;
			}
			
			// Check difficulty range
			if (Definition->MinDifficulty <= CurrentDifficulty)
			{
				if (Definition->MaxDifficulty < 0 || Definition->MaxDifficulty >= CurrentDifficulty)
				{
					ValidDefinitions.Add(Definition);
					TotalWeight += Definition->SelectionWeight;
				}
			}
		}
	}

	if (ValidDefinitions.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: No valid track pieces for difficulty %d, using fallback"), CurrentDifficulty);
		// Fallback to first piece definition if available
		UTrackPieceDefinition* FirstPiece = FindFirstPieceDefinition();
		if (FirstPiece)
		{
			return FirstPiece;
		}
		// Last resort: return first definition in array if any exist
		if (TrackPieceDefinitions.Num() > 0 && TrackPieceDefinitions[0])
		{
			return TrackPieceDefinitions[0];
		}
		return nullptr;
	}

	// Check for zero total weight
	if (TotalWeight <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Total weight is zero, using first valid piece"));
		return ValidDefinitions[0];
	}

	// Get seeded random stream from GameMode
	FRandomStream* RandomStream = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
		{
			RandomStream = &GameMode->GetSeededRandomStream();
		}
	}

	// Weighted random selection using seeded RNG
	int32 RandomWeight = 0;
	if (RandomStream)
	{
		RandomWeight = RandomStream->RandRange(0, TotalWeight - 1);
	}
	else
	{
		// Fallback to global random if GameMode not available
		UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: GameMode not available, using global random"));
		RandomWeight = FMath::RandRange(0, TotalWeight - 1);
	}
	
	int32 CurrentWeight = 0;

	for (UTrackPieceDefinition* Definition : ValidDefinitions)
	{
		CurrentWeight += Definition->SelectionWeight;
		if (RandomWeight < CurrentWeight)
		{
			// Validate selection result
			if (!Definition)
			{
				UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Selected definition is null, using fallback"));
				return ValidDefinitions[0];
			}
			return Definition;
		}
	}

	// Fallback to first valid piece if selection somehow failed
	UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Selection failed, using first valid piece"));
	return ValidDefinitions[0];
}

ATrackPiece* ATrackGenerator::CreateTrackPieceFromDefinition(UTrackPieceDefinition* Definition, const FVector& ConnectionPoint)
{
	// Validate definition
	if (!Definition)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Null definition provided"));
		return nullptr;
	}
	
	if (!TrackPieceClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: TrackPieceClass is null"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Rotate 90 degrees on Yaw axis so pieces face forward along X axis
	FRotator SpawnRotation(0.0f, 90.0f, 0.0f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATrackPiece* NewPiece = nullptr;

	// Validate blueprint class if using blueprint actor
	if (Definition->bUseBlueprintActor && !Definition->BlueprintActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Blueprint actor class is null but bUseBlueprintActor is true"));
		return nullptr;
	}
	
	// Check if we should use a blueprint actor instead of assembling meshes
	if (Definition->bUseBlueprintActor && Definition->BlueprintActorClass)
	{
		// Spawn the blueprint actor at a temporary location first
		// We'll reposition it based on connection points
		FVector TempSpawnLocation = ConnectionPoint;
		NewPiece = World->SpawnActor<ATrackPiece>(Definition->BlueprintActorClass, TempSpawnLocation, SpawnRotation, SpawnParams);
		if (!NewPiece)
		{
			UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to spawn blueprint actor at connection point (%.2f, %.2f, %.2f)"), 
				ConnectionPoint.X, ConnectionPoint.Y, ConnectionPoint.Z);
			return nullptr;
		}

		UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Spawned blueprint actor '%s'"), 
			*Definition->BlueprintActorClass->GetName());
		
		// Build component cache manually (normally done in BeginPlay, but we need it now to find components)
		NewPiece->BuildComponentCache();
		
		// Set connection components by name if specified in data asset
		// This allows designers to explicitly specify which components to use in each blueprint
		if (!Definition->StartConnectionComponentName.IsEmpty())
		{
			NewPiece->SetStartConnectionByName(Definition->StartConnectionComponentName);
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Using StartConnection component '%s'"), *Definition->StartConnectionComponentName);
		}
		else
		{
			// Try to find a component named "StartConnection" in the blueprint
			NewPiece->SetStartConnectionByName(TEXT("StartConnection"));
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Auto-searching for StartConnection component"));
		}
		
		if (Definition->EndConnectionComponentNames.Num() > 0)
		{
			NewPiece->SetEndConnectionsByNames(Definition->EndConnectionComponentNames);
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Using %d EndConnection components from data asset"), Definition->EndConnectionComponentNames.Num());
		}
		else
		{
			// Try to find a component named "EndConnection" in the blueprint
			NewPiece->SetEndConnectionsByNames({ TEXT("EndConnection") });
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Auto-searching for EndConnection component"));
		}
		
		// Position the piece so its start connection aligns with the connection point
		USceneComponent* StartConnection = NewPiece->GetStartConnection();
		if (StartConnection)
		{
			// Get the start connection's current world position (after spawn with rotation)
			FVector StartConnectionWorldPos = StartConnection->GetComponentLocation();
			// Get the actor's current world position
			FVector ActorCurrentPos = NewPiece->GetActorLocation();
			// Calculate the offset from actor to start connection in world space
			FVector ActorToConnection = StartConnectionWorldPos - ActorCurrentPos;
			// Calculate where the actor should be so the connection is at ConnectionPoint
			// We want the start connection to be at (ConnectionPoint.X, 0, ConnectionPoint.Z)
			// So: ActorLocation = ConnectionPoint - ActorToConnection
			// This ensures the connection ends up at ConnectionPoint, and the actor is offset accordingly
			FVector ActorLocation = FVector(ConnectionPoint.X, 0.0f, ConnectionPoint.Z) - ActorToConnection;
			// Force Y to 0 (center lane alignment) - this ensures the connection is at Y=0
			// The actor will be at Y = 0 - ActorToConnection.Y, which is correct
			ActorLocation.Y = 0.0f - ActorToConnection.Y;
			// Preserve Z (ground height) - use ConnectionPoint.Z if provided, otherwise keep current
			if (ConnectionPoint.Z == 0.0f)
			{
				ActorLocation.Z = ActorCurrentPos.Z;
			}
			else
			{
				ActorLocation.Z = ConnectionPoint.Z - ActorToConnection.Z;
			}
			
			NewPiece->SetActorLocation(ActorLocation);
			
			// Verify the connection is actually at Y=0 after positioning
			FVector FinalConnectionPos = NewPiece->GetStartConnectionWorldPosition();
			UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Positioned blueprint actor - Connection offset (%.2f, %.2f, %.2f), Actor at (%.2f, %.2f, %.2f), Connection at (%.2f, %.2f, %.2f)"), 
				ActorToConnection.X, ActorToConnection.Y, ActorToConnection.Z,
				ActorLocation.X, ActorLocation.Y, ActorLocation.Z,
				FinalConnectionPos.X, FinalConnectionPos.Y, FinalConnectionPos.Z);
		}
		else
		{
			// Fallback: center the blueprint actor (X/Y only) if no start connection
			FBox ActorBounds = NewPiece->GetComponentsBoundingBox(true);
			if (ActorBounds.IsValid)
			{
				FVector ActorCenter = ActorBounds.GetCenter();
				FVector ActorOrigin = NewPiece->GetActorLocation();
				FVector Offset = ActorOrigin - ActorCenter;
				
				FVector AdjustedLocation = ConnectionPoint;
				AdjustedLocation.X += Offset.X; // Center on X axis
				AdjustedLocation.Y = 0.0f; // Force Y to 0 (center lane)
				AdjustedLocation.Z = ActorOrigin.Z; // Keep original Z
				
				NewPiece->SetActorLocation(AdjustedLocation);
			}
			else
			{
				// Final fallback: use connection point directly, force Y to 0
				FVector FinalLocation = ConnectionPoint;
				FinalLocation.Y = 0.0f;
				NewPiece->SetActorLocation(FinalLocation);
			}
		}
	}
	else
	{
		// Spawn the base track piece class and assemble meshes
		// Position so start connection aligns with connection point
		FVector TempSpawnLocation = ConnectionPoint;
		NewPiece = World->SpawnActor<ATrackPiece>(TrackPieceClass, TempSpawnLocation, SpawnRotation, SpawnParams);
		if (!NewPiece)
		{
			UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to spawn track piece at connection point (%.2f, %.2f, %.2f)"), 
				ConnectionPoint.X, ConnectionPoint.Y, ConnectionPoint.Z);
			return nullptr;
		}
		
		// Position the piece so its start connection aligns with the connection point
		USceneComponent* StartConnection = NewPiece->GetStartConnection();
		if (StartConnection)
		{
			// Get the start connection's current world position (after spawn with rotation)
			FVector StartConnectionWorldPos = StartConnection->GetComponentLocation();
			// Get the actor's current world position
			FVector ActorCurrentPos = NewPiece->GetActorLocation();
			// Calculate the offset from actor to start connection in world space
			FVector ActorToConnection = StartConnectionWorldPos - ActorCurrentPos;
			// Calculate where the actor should be so the connection is at ConnectionPoint
			// We want the start connection to be at (ConnectionPoint.X, 0, ConnectionPoint.Z)
			// So: ActorLocation = ConnectionPoint - ActorToConnection
			FVector ActorLocation = FVector(ConnectionPoint.X, 0.0f, ConnectionPoint.Z) - ActorToConnection;
			// Force Y so connection is at Y=0: ActorLocation.Y = 0 - ActorToConnection.Y
			ActorLocation.Y = 0.0f - ActorToConnection.Y;
			
			NewPiece->SetActorLocation(ActorLocation);
		}
		else
		{
			// Fallback: use connection point directly, force Y to 0
			FVector FinalLocation = ConnectionPoint;
			FinalLocation.Y = 0.0f;
			NewPiece->SetActorLocation(FinalLocation);
		}
		
		UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Spawned track piece - Start connection at (%.2f, %.2f, %.2f)"), 
			NewPiece->GetStartConnectionWorldPosition().X,
			NewPiece->GetStartConnectionWorldPosition().Y,
			NewPiece->GetStartConnectionWorldPosition().Z);
	}

	// Validate spawn succeeded
	if (!NewPiece)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to spawn track piece"));
		return nullptr;
	}

	// Set length
	NewPiece->SetLength(Definition->Length);

	// Only assemble meshes if not using a blueprint actor
	if (!Definition->bUseBlueprintActor)
	{
		// Create mesh components from definition
		USceneComponent* RootSceneComp = NewPiece->GetRootSceneComponent();
		if (RootSceneComp)
	{
		// Create all components first, then set properties, then register all at once
		// This prevents the "pop-in" effect where components appear sequentially
		UStaticMeshComponent* FloorComponent = nullptr;
		UStaticMeshComponent* LeftWallComponent = nullptr;
		UStaticMeshComponent* RightWallComponent = nullptr;
		UStaticMeshComponent* CeilingComponent = nullptr;
		
		UStaticMesh* FloorMeshAsset = nullptr;
		UStaticMesh* LeftWallMesh = nullptr;
		UStaticMesh* RightWallMesh = nullptr;
		UStaticMesh* CeilingMesh = nullptr;
		
		// Floor mesh - position to align with lanes (this is the "boss" - walls attach to its sockets)
		if (Definition->Meshes.FloorMesh)
		{
			FloorMeshAsset = Definition->Meshes.FloorMesh;
			if (FloorMeshAsset)
			{
				FloorComponent = NewObject<UStaticMeshComponent>(NewPiece, UStaticMeshComponent::StaticClass(), TEXT("FloorMesh"));
				FloorComponent->SetupAttachment(RootSceneComp);
				FloorComponent->SetStaticMesh(FloorMeshAsset);
				
				// Get mesh bounds to center it on lanes
				FBoxSphereBounds MeshBounds = FloorMeshAsset->GetBounds();
				FVector MeshExtent = MeshBounds.BoxExtent;
				
				// Center the mesh at the root by offsetting by negative origin
				FVector FloorOffset(-MeshBounds.Origin.X, -MeshBounds.Origin.Y, -MeshBounds.Origin.Z);
				FloorComponent->SetRelativeLocation(FloorOffset);
				
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Floor mesh offset to (%.2f, %.2f, %.2f) to center mesh"), 
					FloorOffset.X, FloorOffset.Y, FloorOffset.Z);
				
				UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Floor mesh bounds - Origin: (%.2f, %.2f, %.2f), Extent: (%.2f, %.2f, %.2f)"), 
					MeshBounds.Origin.X, MeshBounds.Origin.Y, MeshBounds.Origin.Z,
					MeshExtent.X, MeshExtent.Y, MeshExtent.Z);
			}
		}

		// Left wall - attach to floor mesh socket if available, otherwise use calculated position
		if (Definition->Meshes.LeftWallMesh)
		{
			LeftWallMesh = Definition->Meshes.LeftWallMesh;
			if (LeftWallMesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Creating left wall mesh component"));
				LeftWallComponent = NewObject<UStaticMeshComponent>(NewPiece, UStaticMeshComponent::StaticClass(), TEXT("LeftWallMesh"));
				
				// Try to attach to floor mesh socket first
				bool bAttachedToSocket = false;
				if (FloorComponent && FloorMeshAsset)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent and FloorMeshAsset exist, checking for Left_Wall socket"));
					
					// Check if floor mesh has Left_Wall socket (can check on mesh asset directly)
					if (const UStaticMeshSocket* Socket = FloorMeshAsset->FindSocket(TEXT("Left_Wall")))
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Found Left_Wall socket!"));
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Socket relative location: (%.2f, %.2f, %.2f)"), 
							Socket->RelativeLocation.X, Socket->RelativeLocation.Y, Socket->RelativeLocation.Z);
						
						// Attach to socket but use position only, reset rotation and scale
						LeftWallComponent->SetupAttachment(FloorComponent, TEXT("Left_Wall"));
						LeftWallComponent->SetRelativeRotation(FRotator::ZeroRotator);
						LeftWallComponent->SetRelativeScale3D(FVector::OneVector);
						bAttachedToSocket = true;
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Left wall attached to floor mesh 'Left_Wall' socket (rotation and scale reset)"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Left_Wall socket NOT found in floor mesh"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent or FloorMeshAsset is NULL - FloorComponent: %s, FloorMeshAsset: %s"), 
						FloorComponent ? TEXT("VALID") : TEXT("NULL"),
						FloorMeshAsset ? TEXT("VALID") : TEXT("NULL"));
				}
				
				// Fall back to root attachment with calculated position
				if (!bAttachedToSocket)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Using calculated position for left wall"));
					LeftWallComponent->SetupAttachment(RootSceneComp);
					// Position left wall at left lane edge (Y = -200)
					FBoxSphereBounds WallBounds = LeftWallMesh->GetBounds();
					FVector LeftWallOffset(0.0f, -200.0f - WallBounds.Origin.Y, 0.0f);
					LeftWallComponent->SetRelativeLocation(LeftWallOffset);
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Left wall positioned at calculated offset (%.2f, %.2f, %.2f)"), 
						LeftWallOffset.X, LeftWallOffset.Y, LeftWallOffset.Z);
					
					// Draw debug sphere at calculated position
					if (World)
					{
						FVector WorldPos = NewPiece->GetActorLocation() + LeftWallOffset;
						DrawDebugSphere(World, WorldPos, 50.0f, 12, FColor::Orange, false, 10.0f, 0, 3.0f);
					}
				}
				
				LeftWallComponent->SetStaticMesh(LeftWallMesh);
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Left wall mesh set: %s"), LeftWallMesh ? *LeftWallMesh->GetName() : TEXT("NULL"));
				
				// Apply flip if requested (flip on Y axis to mirror horizontally)
				if (Definition->Meshes.bFlipLeftWall)
				{
					LeftWallComponent->SetRelativeScale3D(FVector(1.0f, -1.0f, 1.0f));
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Left wall flipped horizontally"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to load LeftWallMesh"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: LeftWallMesh is not valid in definition"));
		}

		// Right wall - attach to floor mesh socket if available, otherwise use calculated position
		if (Definition->Meshes.RightWallMesh)
		{
			RightWallMesh = Definition->Meshes.RightWallMesh;
			if (RightWallMesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Creating right wall mesh component"));
				RightWallComponent = NewObject<UStaticMeshComponent>(NewPiece, UStaticMeshComponent::StaticClass(), TEXT("RightWallMesh"));
				
				// Try to attach to floor mesh socket first
				bool bAttachedToSocket = false;
				if (FloorComponent && FloorMeshAsset)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent and FloorMeshAsset exist, checking for Right_Wall socket"));
					
					// Check if floor mesh has Right_Wall socket (can check on mesh asset directly)
					if (const UStaticMeshSocket* Socket = FloorMeshAsset->FindSocket(TEXT("Right_Wall")))
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Found Right_Wall socket!"));
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Socket relative location: (%.2f, %.2f, %.2f)"), 
							Socket->RelativeLocation.X, Socket->RelativeLocation.Y, Socket->RelativeLocation.Z);
						
						// Attach to socket but use position only, reset rotation and scale
						RightWallComponent->SetupAttachment(FloorComponent, TEXT("Right_Wall"));
						RightWallComponent->SetRelativeRotation(FRotator::ZeroRotator);
						RightWallComponent->SetRelativeScale3D(FVector::OneVector);
						bAttachedToSocket = true;
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Right wall attached to floor mesh 'Right_Wall' socket (rotation and scale reset)"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Right_Wall socket NOT found in floor mesh"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent or FloorMeshAsset is NULL - FloorComponent: %s, FloorMeshAsset: %s"), 
						FloorComponent ? TEXT("VALID") : TEXT("NULL"),
						FloorMeshAsset ? TEXT("VALID") : TEXT("NULL"));
				}
				
				// Fall back to root attachment with calculated position
				if (!bAttachedToSocket)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Using calculated position for right wall"));
					RightWallComponent->SetupAttachment(RootSceneComp);
					// Position right wall at right lane edge (Y = 200)
					FBoxSphereBounds WallBounds = RightWallMesh->GetBounds();
					FVector RightWallOffset(0.0f, 200.0f - WallBounds.Origin.Y, 0.0f);
					RightWallComponent->SetRelativeLocation(RightWallOffset);
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Right wall positioned at calculated offset (%.2f, %.2f, %.2f)"), 
						RightWallOffset.X, RightWallOffset.Y, RightWallOffset.Z);
					
					// Draw debug sphere at calculated position
					if (World)
					{
						FVector WorldPos = NewPiece->GetActorLocation() + RightWallOffset;
						DrawDebugSphere(World, WorldPos, 50.0f, 12, FColor::Cyan, false, 10.0f, 0, 3.0f);
					}
				}
				
				RightWallComponent->SetStaticMesh(RightWallMesh);
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Right wall mesh set: %s"), RightWallMesh ? *RightWallMesh->GetName() : TEXT("NULL"));
				
				// Apply flip if requested (flip on Y axis to mirror horizontally)
				if (Definition->Meshes.bFlipRightWall)
				{
					RightWallComponent->SetRelativeScale3D(FVector(1.0f, -1.0f, 1.0f));
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Right wall flipped horizontally"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to load RightWallMesh"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: RightWallMesh is not valid in definition"));
		}

		// Ceiling (optional) - use Ceiling socket from floor mesh for X, Y, Z positioning
		if (Definition->Meshes.CeilingMesh)
		{
			CeilingMesh = Definition->Meshes.CeilingMesh;
			if (CeilingMesh)
			{
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Creating ceiling mesh component"));
				CeilingComponent = NewObject<UStaticMeshComponent>(NewPiece, UStaticMeshComponent::StaticClass(), TEXT("CeilingMesh"));
				
				// Try to use floor mesh Ceiling socket for X, Y, Z positioning
				bool bUsedSocket = false;
				if (FloorComponent && FloorMeshAsset)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent and FloorMeshAsset exist, checking for Ceiling socket"));
					
					// Check if floor mesh has Ceiling socket (can check on mesh asset directly)
					if (const UStaticMeshSocket* Socket = FloorMeshAsset->FindSocket(TEXT("Ceiling")))
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Found Ceiling socket!"));
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Socket relative location: (%.2f, %.2f, %.2f)"), 
							Socket->RelativeLocation.X, Socket->RelativeLocation.Y, Socket->RelativeLocation.Z);
						
						// Calculate relative location from socket (relative to floor component, then convert to root-relative)
						// Socket is relative to floor component, so we need to add floor component's relative location
						FVector FloorRelativeLoc = FloorComponent->GetRelativeLocation();
						FVector RelativeLocation = Socket->RelativeLocation + FloorRelativeLoc;
						
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Ceiling relative location (using socket X,Y,Z): (%.2f, %.2f, %.2f)"), 
							RelativeLocation.X, RelativeLocation.Y, RelativeLocation.Z);
						
						CeilingComponent->SetupAttachment(RootSceneComp);
						CeilingComponent->SetRelativeLocation(RelativeLocation);
						CeilingComponent->SetRelativeRotation(FRotator::ZeroRotator);
						CeilingComponent->SetRelativeScale3D(FVector::OneVector);
						bUsedSocket = true;
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Ceiling positioned using floor mesh 'Ceiling' socket (X,Y,Z)"));
					}
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Ceiling socket NOT found in floor mesh"));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: FloorComponent or FloorMeshAsset is NULL - FloorComponent: %s, FloorMeshAsset: %s"), 
						FloorComponent ? TEXT("VALID") : TEXT("NULL"),
						FloorMeshAsset ? TEXT("VALID") : TEXT("NULL"));
				}
				
				// Fall back to root attachment at origin if socket not found
				if (!bUsedSocket)
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Using default position for ceiling (0,0,0 relative to root)"));
					CeilingComponent->SetupAttachment(RootSceneComp);
					CeilingComponent->SetRelativeLocation(FVector::ZeroVector);
				}
				
				CeilingComponent->SetStaticMesh(CeilingMesh);
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Ceiling mesh set: %s"), CeilingMesh ? *CeilingMesh->GetName() : TEXT("NULL"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to load CeilingMesh"));
			}
		}
		
		// Register all components at once to prevent sequential "pop-in" effect
		// Register floor first (walls depend on its sockets)
		if (FloorComponent)
		{
			FloorComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Floor component registered"));
		}
		
		// Register walls (they attach to floor sockets)
		if (LeftWallComponent)
		{
			LeftWallComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Left wall component registered. Final location: (%.2f, %.2f, %.2f)"), 
				LeftWallComponent->GetComponentLocation().X, 
				LeftWallComponent->GetComponentLocation().Y, 
				LeftWallComponent->GetComponentLocation().Z);
		}
		
		if (RightWallComponent)
		{
			RightWallComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Right wall component registered. Final location: (%.2f, %.2f, %.2f)"), 
				RightWallComponent->GetComponentLocation().X, 
				RightWallComponent->GetComponentLocation().Y, 
				RightWallComponent->GetComponentLocation().Z);
		}
		
		// Register ceiling last
		if (CeilingComponent)
		{
			CeilingComponent->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Ceiling component registered. Final location: (%.2f, %.2f, %.2f)"), 
				CeilingComponent->GetComponentLocation().X, 
				CeilingComponent->GetComponentLocation().Y, 
				CeilingComponent->GetComponentLocation().Z);
		}
		}
	}

	// Add spawn points from definition
	for (const FTrackPieceSpawnConfig& Config : Definition->SpawnConfigs)
	{
		// Check spawn probability using seeded RNG
		float RandomValue = 0.0f;
		if (UWorld* WorldPtr = GetWorld())
		{
			if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(WorldPtr->GetAuthGameMode()))
			{
				RandomValue = GameMode->GetSeededRandomStream().FRand();
			}
			else
			{
				// Fallback to global random if GameMode not available
				RandomValue = FMath::RandRange(0.0f, 1.0f);
			}
		}
		else
		{
			RandomValue = FMath::RandRange(0.0f, 1.0f);
		}
		
		if (RandomValue <= Config.SpawnProbability)
		{
			FSpawnPoint SpawnPoint;
			SpawnPoint.Lane = Config.Lane;
			SpawnPoint.ForwardPosition = Config.ForwardPosition;
			SpawnPoint.SpawnType = Config.SpawnType;
			SpawnPoint.SpawnClass = Config.SpawnClass;
			SpawnPoint.SpawnPositionComponentName = Config.SpawnPositionComponentName;
			NewPiece->AddSpawnPoint(SpawnPoint);
		}
	}

	// Spawn collectibles and obstacles on the track piece
	// Get GameMode -> GameplayManager -> SpawnManager to spawn actors
	if (UWorld* WorldPtr = GetWorld())
	{
		if (AEndlessRunnerGameMode* GameMode = Cast<AEndlessRunnerGameMode>(WorldPtr->GetAuthGameMode()))
		{
			if (UGameplayManager* GameplayManager = GameMode->GetGameplayManager())
			{
				if (USpawnManager* SpawnManager = GameplayManager->GetSpawnManager())
				{
					SpawnManager->SpawnOnTrackPiece(NewPiece);
					UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Called SpawnOnTrackPiece for piece '%s' with %d spawn points"), 
						*NewPiece->GetName(), NewPiece->GetSpawnPoints().Num());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: SpawnManager is null, cannot spawn actors on track piece"));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: GameplayManager is null, cannot spawn actors"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: GameMode is not AEndlessRunnerGameMode, cannot spawn actors"));
		}
	}

	// Final validation: ensure piece was created successfully
	if (!NewPiece)
	{
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to spawn track piece - NewPiece is null"));
		return nullptr;
	}

	return NewPiece;
}

void ATrackGenerator::DrawLaneDebugVisualization()
{
	if (!GetWorld())
	{
		return;
	}

	// Fixed lane coordinates (matching RabbitCharacter)
	constexpr float LANE_LEFT_Y = -200.0f;
	constexpr float LANE_CENTER_Y = 0.0f;
	constexpr float LANE_RIGHT_Y = 200.0f;
	constexpr float LANE_WIDTH = 200.0f;
	constexpr float LANE_HALF_WIDTH = LANE_WIDTH * 0.5f;

	// Draw distance ahead and behind player
	float DrawStart = DistanceTraveled - 2000.0f;
	float DrawEnd = DistanceTraveled + 5000.0f;
	float DrawLength = DrawEnd - DrawStart;

	// Draw left lane (red)
	FVector LeftLaneStart(DrawStart, LANE_LEFT_Y, 10.0f);
	FVector LeftLaneEnd(DrawEnd, LANE_LEFT_Y, 10.0f);
	FVector LeftLaneSize(DrawLength, LANE_HALF_WIDTH, 5.0f);
	FVector LeftLaneCenter = (LeftLaneStart + LeftLaneEnd) * 0.5f;
	DrawDebugBox(GetWorld(), LeftLaneCenter, LeftLaneSize * 0.5f, FColor::Red, false, 0.0f, 0, 5.0f);

	// Draw center lane (green)
	FVector CenterLaneStart(DrawStart, LANE_CENTER_Y, 10.0f);
	FVector CenterLaneEnd(DrawEnd, LANE_CENTER_Y, 10.0f);
	FVector CenterLaneSize(DrawLength, LANE_HALF_WIDTH, 5.0f);
	FVector CenterLaneCenter = (CenterLaneStart + CenterLaneEnd) * 0.5f;
	DrawDebugBox(GetWorld(), CenterLaneCenter, CenterLaneSize * 0.5f, FColor::Green, false, 0.0f, 0, 5.0f);

	// Draw right lane (blue)
	FVector RightLaneStart(DrawStart, LANE_RIGHT_Y, 10.0f);
	FVector RightLaneEnd(DrawEnd, LANE_RIGHT_Y, 10.0f);
	FVector RightLaneSize(DrawLength, LANE_HALF_WIDTH, 5.0f);
	FVector RightLaneCenter = (RightLaneStart + RightLaneEnd) * 0.5f;
	DrawDebugBox(GetWorld(), RightLaneCenter, RightLaneSize * 0.5f, FColor::Blue, false, 0.0f, 0, 5.0f);

	// Draw lane center lines for clarity
	DrawDebugLine(GetWorld(), LeftLaneStart, LeftLaneEnd, FColor::Red, false, 0.0f, 0, 2.0f);
	DrawDebugLine(GetWorld(), CenterLaneStart, CenterLaneEnd, FColor::Green, false, 0.0f, 0, 2.0f);
	DrawDebugLine(GetWorld(), RightLaneStart, RightLaneEnd, FColor::Blue, false, 0.0f, 0, 2.0f);
}

