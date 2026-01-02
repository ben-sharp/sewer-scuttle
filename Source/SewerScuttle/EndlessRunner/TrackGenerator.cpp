// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrackGenerator.h"
#include "TrackPiece.h"
#include "TrackPieceDefinition.h"
#include "ContentRegistry.h"
#include "RabbitCharacter.h"
#include "EndlessRunnerGameMode.h"
#include "GameplayManager.h"
#include "SpawnManager.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetRegistry/AssetRegistryModule.h"

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
	if (!PlayerCharacter) return;
	DistanceTraveled = PlayerCharacter->GetActorLocation().X;

	// Check for reached pieces (Shops/Bosses)
	for (ATrackPiece* Piece : ActiveTrackPieces)
	{
		if (Piece && !ReachedPieces.Contains(Piece))
		{
			// Check if player has reached the middle of the piece
			if (DistanceTraveled >= Piece->GetActorLocation().X)
			{
				ReachedPieces.Add(Piece);
				
				FString* PieceId = PieceIdMap.Find(Piece);
				if (PieceId)
				{
					UTrackPieceDefinition* D = FindTrackPieceDefinitionById(*PieceId);
					if (D)
					{
						if (D->PieceType == ETrackPieceType::Shop)
						{
							UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Player reached SHOP piece: %s"), **PieceId);
							OnShopPieceReached.Broadcast();
						}
						else if (D->PieceType == ETrackPieceType::Boss)
						{
							UE_LOG(LogTemp, Warning, TEXT("TrackGenerator: Player reached BOSS piece: %s"), **PieceId);
							OnBossPieceReached.Broadcast();
						}
					}
				}
			}
		}
	}

	SpawnCheckTimer += DeltaTime;
	if (SpawnCheckTimer >= SpawnCheckInterval)
	{
		SpawnCheckTimer = 0.0f;
		float DistanceAhead = DistanceTraveled + 40000.0f; // Increase spawn ahead distance to 50m (40,000 units)
		if (bTrackSequenceLoaded && !bEndlessMode)
		{
			if (LastSpawnPosition < DistanceAhead && CurrentPieceIndex < TrackSequenceData.Pieces.Num())
				SpawnNextSequencePiece();
		}
		else if (bEndlessMode)
		{
			if (LastSpawnPosition < DistanceAhead)
				SpawnTrackPiece(LastSpawnPosition);
		}
	}
	static float CleanupTimer = 0.0f;
	CleanupTimer += DeltaTime;
	if (CleanupTimer >= 1.0f) { CleanupTimer = 0.0f; CleanupOldPieces(); }
}

void ATrackGenerator::Initialize(ARabbitCharacter* InPlayerCharacter)
{
	PlayerCharacter = InPlayerCharacter;
	if (TrackPieceDefinitions.Num() == 0 || !TrackPieceClass) return;
	
	// If we have a sequence, we should clear everything and start from the sequence
	if (bTrackSequenceLoaded)
	{
		Reset();
		bTrackSequenceLoaded = true; // Reset clears this, so set it back
		CurrentPieceIndex = 0;
		LastSpawnPosition = 0.0f;
		
		// Pre-spawn initial pieces from sequence
		// Ensure the first piece (index 0) is actually spawned at 0
		for (int32 i = 0; i < PiecesAhead && CurrentPieceIndex < TrackSequenceData.Pieces.Num(); ++i)
		{
			SpawnNextSequencePiece();
		}
		
		// If we still haven't spawned anything (sequence empty or something), fallback to standard
		if (ActiveTrackPieces.Num() > 0)
		{
			return;
		}
	}

	FVector FirstConnectionPoint(0.0f, 0.0f, 0.0f);
	LastSpawnPosition = 0.0f;
	UTrackPieceDefinition* FirstPieceDefinition = FindFirstPieceDefinition();
	
	for (int32 i = 0; i < PiecesAhead; ++i)
	{
		FVector CP = (i == 0) ? FirstConnectionPoint : FVector(LastSpawnPosition, 0.0f, 0.0f);
		if (i == 0 && FirstPieceDefinition)
		{
			ATrackPiece* NP = CreateTrackPieceFromDefinition(FirstPieceDefinition, CP);
			if (NP) { ActiveTrackPieces.Add(NP); LastSpawnPosition = NP->GetEndConnectionWorldPosition().X; }
			else SpawnTrackPiece(CP.X);
		}
		else
		{
			if (ActiveTrackPieces.Num() > 0)
			{
				ATrackPiece* LP = ActiveTrackPieces.Last();
				if (LP) { FVector ECP = LP->GetEndConnectionWorldPosition(); CP = FVector(ECP.X, 0.0f, 0.0f); LastSpawnPosition = ECP.X; }
			}
			SpawnTrackPiece(LastSpawnPosition);
		}
	}
}

void ATrackGenerator::UpdatePlayerReference(ARabbitCharacter* InPlayerCharacter) { PlayerCharacter = InPlayerCharacter; }

void ATrackGenerator::Reset()
{
	for (ATrackPiece* P : ActiveTrackPieces) if (IsValid(P)) { P->ClearSpawnedActors(); P->Destroy(); }
	ActiveTrackPieces.Empty(); PieceIdMap.Empty(); ReachedPieces.Empty();
	TotalTrackPiecesSpawned = 0; LastSpawnPosition = 0.0f; DistanceTraveled = 0.0f; PlayerCharacter = nullptr; CurrentPieceIndex = 0; bTrackSequenceLoaded = false;
}

void ATrackGenerator::LoadTrackSequence(const FTrackSequenceData& SequenceData)
{
	Reset(); TrackSequenceData = SequenceData; CurrentPieceIndex = 0; bTrackSequenceLoaded = true; bEndlessMode = false; LastSpawnPosition = 0.0f;
}

int32 ATrackGenerator::GetRemainingPieces() const { return bTrackSequenceLoaded ? FMath::Max(0, TrackSequenceData.Pieces.Num() - CurrentPieceIndex) : 0; }

TArray<FString> ATrackGenerator::GetCurrentPieceIds() const
{
    TArray<FString> Ids;
    for (const FTrackPiecePrescription& P : TrackSequenceData.Pieces) Ids.Add(P.PieceId);
    return Ids;
}

UTrackPieceDefinition* ATrackGenerator::FindTrackPieceDefinitionById(const FString& ContentId) const
{
	if (UWorld* World = GetWorld())
	{
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(World->GetAuthGameMode()))
		{
			if (UContentRegistry* Registry = GM->GetContentRegistry())
			{
				return Registry->FindTrackPieceById(ContentId);
			}
		}
	}
	
	// Fallback to local list if GameMode registry is not available
	for (UTrackPieceDefinition* D : TrackPieceDefinitions) if (D && D->GetName() == ContentId) return D;
	return nullptr;
}

void ATrackGenerator::SpawnNextSequencePiece()
{
	if (!bTrackSequenceLoaded || CurrentPieceIndex >= TrackSequenceData.Pieces.Num()) return;
	FString PieceId = TrackSequenceData.Pieces[CurrentPieceIndex].PieceId;
	UTrackPieceDefinition* D = FindTrackPieceDefinitionById(PieceId);
	if (!D) { 
		UE_LOG(LogTemp, Error, TEXT("TrackGenerator: Failed to find definition for PieceId: %s"), *PieceId);
		CurrentPieceIndex++; 
		return; 
	}

	FVector CP(LastSpawnPosition, 0.0f, 0.0f);
	if (ActiveTrackPieces.Num() > 0)
	{
		ATrackPiece* LP = ActiveTrackPieces.Last();
		if (LP) CP = FVector(LP->GetEndConnectionWorldPosition().X, 0.0f, 0.0f);
	}
	else
	{
		// Force the very first piece to spawn at world 0,0,0
		CP = FVector::ZeroVector;
	}

	ATrackPiece* NP = CreateTrackPieceFromDefinition(D, CP);
	if (NP) { 
		ActiveTrackPieces.Add(NP); 
		TotalTrackPiecesSpawned++; 
		LastSpawnPosition = NP->GetEndConnectionWorldPosition().X; 
		PieceIdMap.Add(NP, PieceId); 
        
        // Pass prescribed spawns to the piece
        NP->SetPrescribedSpawns(TrackSequenceData.Pieces[CurrentPieceIndex].PrescribedSpawns);
        
		CurrentPieceIndex++; 
		
		UE_LOG(LogTemp, Log, TEXT("TrackGenerator: Spawned sequence piece %d: %s at X=%.2f"), 
			CurrentPieceIndex-1, *PieceId, CP.X);
	}
	else {
		CurrentPieceIndex++;
	}
}

void ATrackGenerator::SpawnTrackPiece(float Position)
{
	UTrackPieceDefinition* D = SelectTrackPieceDefinition();
	if (!D) return;
	FVector CP(Position, 0.0f, 0.0f);
	if (ActiveTrackPieces.Num() > 0)
	{
		ATrackPiece* LP = ActiveTrackPieces.Last();
		if (LP) CP = FVector(LP->GetEndConnectionWorldPosition().X, 0.0f, 0.0f);
	}
	ATrackPiece* NP = CreateTrackPieceFromDefinition(D, CP);
	if (NP) { ActiveTrackPieces.Add(NP); TotalTrackPiecesSpawned++; LastSpawnPosition = NP->GetEndConnectionWorldPosition().X; }
}

void ATrackGenerator::CleanupOldPieces()
{
	float DP = DistanceTraveled - DestroyDistanceBehind;
	for (int32 i = ActiveTrackPieces.Num() - 1; i >= 0; --i)
	{
		ATrackPiece* P = ActiveTrackPieces[i];
		if (P && P->GetActorLocation().X < DP) { P->ClearSpawnedActors(); P->Destroy(); ActiveTrackPieces.RemoveAt(i); PieceIdMap.Remove(P); }
	}
}

UTrackPieceDefinition* ATrackGenerator::FindFirstPieceDefinition() const
{
	for (UTrackPieceDefinition* D : TrackPieceDefinitions) if (D && D->PieceType == ETrackPieceType::Start) return D;
	return nullptr;
}

UTrackPieceDefinition* ATrackGenerator::SelectTrackPieceDefinition()
{
	TArray<UTrackPieceDefinition*> Valid; int32 TW = 0;
	for (UTrackPieceDefinition* D : TrackPieceDefinitions)
	{
		if (!D) continue;
		
		// Start pieces are never picked randomly (only used at sequence start or initialization)
		if (D->PieceType == ETrackPieceType::Start) continue;

		bool bValidType = (D->PieceType == ETrackPieceType::Normal);
		if (bEndlessMode && D->PieceType == ETrackPieceType::Boss) bValidType = true;
		
		if (bValidType && D->MinDifficulty <= CurrentDifficulty && (D->MaxDifficulty < 0 || D->MaxDifficulty >= CurrentDifficulty))
		{ Valid.Add(D); TW += D->SelectionWeight; }
	}
	if (Valid.Num() == 0) 
	{
		// Fallback logic
		if (TrackPieceDefinitions.Num() > 0) return TrackPieceDefinitions[0];
		return nullptr;
	}
	if (TW <= 0) return Valid[0];
	FRandomStream* RS = nullptr;
	if (UWorld* W = GetWorld()) if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(W->GetAuthGameMode())) RS = &GM->GetSeededRandomStream();
	int32 RW = RS ? RS->RandRange(0, TW - 1) : FMath::RandRange(0, TW - 1);
	int32 CW = 0;
	for (UTrackPieceDefinition* D : Valid) { CW += D->SelectionWeight; if (RW < CW) return D; }
	return Valid[0];
}

ATrackPiece* ATrackGenerator::CreateTrackPieceFromDefinition(UTrackPieceDefinition* D, const FVector& CP)
{
	if (!D || !TrackPieceClass) return nullptr;
	UWorld* W = GetWorld(); if (!W) return nullptr;
	FRotator SR(0.0f, 90.0f, 0.0f); FActorSpawnParameters SP; SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATrackPiece* NP = nullptr;

	if (D->bUseBlueprintActor && D->BlueprintActorClass)
	{
		NP = W->SpawnActor<ATrackPiece>(D->BlueprintActorClass, CP, SR, SP);
		if (!NP) return nullptr;
		NP->BuildComponentCache();
		NP->SetStartConnectionByName(D->StartConnectionComponentName.IsEmpty() ? TEXT("StartConnection") : D->StartConnectionComponentName);
		NP->SetEndConnectionsByNames(D->EndConnectionComponentNames.Num() > 0 ? D->EndConnectionComponentNames : TArray<FString>{TEXT("EndConnection")});
		if (USceneComponent* SC = NP->GetStartConnection())
		{
			FVector SCP = SC->GetComponentLocation(), ACP = NP->GetActorLocation(), ATC = SCP - ACP;
			FVector AL = FVector(CP.X, 0.0f, CP.Z == 0.0f ? ACP.Z : CP.Z) - ATC;
			AL.Y = 0.0f - ATC.Y; if (CP.Z != 0.0f) AL.Z = CP.Z - ATC.Z;
			NP->SetActorLocation(AL);
		}
	}
	else
	{
		NP = W->SpawnActor<ATrackPiece>(TrackPieceClass, CP, SR, SP);
		if (!NP) return nullptr;
		if (USceneComponent* SC = NP->GetStartConnection())
		{
			FVector SCP = SC->GetComponentLocation(), ACP = NP->GetActorLocation(), ATC = SCP - ACP;
			FVector AL = FVector(CP.X, 0.0f, ACP.Z) - ATC; AL.Y = 0.0f - ATC.Y;
			NP->SetActorLocation(AL);
		}
	}

	if (NP)
	{
		NP->SetLength(D->Length);
		NP->SetLaneWidth(D->LaneWidth);
		if (AEndlessRunnerGameMode* GM = Cast<AEndlessRunnerGameMode>(W->GetAuthGameMode())) if (UGameplayManager* GPM = GM->GetGameplayManager()) if (USpawnManager* SPM = GPM->GetSpawnManager()) SPM->SpawnOnTrackPiece(NP);
	}
	return NP;
}

void ATrackGenerator::DrawLaneDebugVisualization() {}
