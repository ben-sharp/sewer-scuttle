// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "PlayerClass.h"
#include "WebServerInterface.generated.h"

/** Run seed data from server */
USTRUCT(BlueprintType)
struct FRunSeedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SeedId;

	UPROPERTY(BlueprintReadWrite)
	int32 Seed = 0;

	UPROPERTY(BlueprintReadWrite)
	FString ContentVersion;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxCoins = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxObstacles = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxTrackPieces = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxDistance = 0;
};

/** Track info for selection */
USTRUCT(BlueprintType)
struct FTrackInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 Id = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 Length = 0;

    UPROPERTY(BlueprintReadWrite)
    int32 ShopCount = 0;

    UPROPERTY(BlueprintReadWrite)
    FString BossId;
};

/** Track selection data (from /runs/start) */
USTRUCT(BlueprintType)
struct FTrackSelectionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SeedId;

    UPROPERTY(BlueprintReadWrite)
    int32 Seed = 0;
    
    UPROPERTY(BlueprintReadWrite)
    FString ContentVersion;

	UPROPERTY(BlueprintReadWrite)
	int32 Tier = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FTrackInfo> Tracks;
};

/** Track sequence data (from /runs/select-track) */
USTRUCT(BlueprintType)
struct FTrackSequenceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> PieceIds;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> ShopPositions;

	UPROPERTY(BlueprintReadWrite)
	FString BossId;

	UPROPERTY(BlueprintReadWrite)
	int32 Length = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 ShopCount = 0;
};

/** Shop item data */
USTRUCT(BlueprintType)
struct FShopItemData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Id;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	int32 Cost = 0;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> Properties;
};

/** Shop data response */
USTRUCT(BlueprintType)
struct FShopData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FShopItemData> Items;
};

/** Boss reward data */
USTRUCT(BlueprintType)
struct FBossRewardData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Id;

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> Properties;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSeedReceived, const FRunSeedData&, SeedData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTrackSelectionReceived, const FTrackSelectionData&, SelectionData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTrackSequenceReceived, const FTrackSequenceData&, SequenceData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnShopItemsReceived, const FShopData&, ShopData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBossRewardsReceived, const TArray<FBossRewardData>&, Rewards);

/**
 * Handles communication with the backend web server
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UWebServerInterface : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize the interface */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void Initialize();

	/** Request a new run seed */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void RequestRunSeed(int32 MaxDistance, EPlayerClass PlayerClass);

	/** Submit a completed run */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SubmitRun(const FString& SeedId, int32 Score, int32 Distance, int32 DurationSeconds,
		int32 CoinsCollected, int32 ObstaclesHit, int32 PowerupsUsed, int32 TrackPiecesSpawned,
		const FString& StartedAt, const TArray<int32>& SelectedTracks, bool bIsComplete, bool bIsEndless,
		const TArray<FString>& PieceSequence);

	/** Select a track for the current tier */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SelectTrack(const FString& SeedId, int32 Tier, int32 TrackIndex);

	/** Get items for a shop */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void GetShopItems(const FString& SeedId, int32 Tier, int32 TrackIndex, int32 ShopIndex);

	/** Reroll items for a shop */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void RerollShop(const FString& SeedId, int32 Tier, int32 TrackIndex, int32 ShopIndex);

	/** Get reward options for defeating a boss */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void GetBossRewards(const FString& SeedId, int32 Tier);

    /** Request track selection for a specific tier */
    UFUNCTION(BlueprintCallable, Category = "Web Server")
    void RequestTierTracks(const FString& SeedId, int32 Tier);

	/** Save currency to server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SaveCurrency(int32 Amount);

	/** Load currency from server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void LoadCurrency();

	/** Purchase item from server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void PurchaseItem(const FString& ItemId, int32 Price);

	/** Delegate setters */
	void SetOnSeedReceived(FOnSeedReceived Delegate) { OnSeedReceived = Delegate; }
	void SetOnError(FOnError Delegate) { OnError = Delegate; }
	void SetOnTrackSelectionReceived(FOnTrackSelectionReceived Delegate) { OnTrackSelectionReceived = Delegate; }
	void SetOnTrackSequenceReceived(FOnTrackSequenceReceived Delegate) { OnTrackSequenceReceived = Delegate; }
	void SetOnShopItemsReceived(FOnShopItemsReceived Delegate) { OnShopItemsReceived = Delegate; }
	void SetOnBossRewardsReceived(FOnBossRewardsReceived Delegate) { OnBossRewardsReceived = Delegate; }

protected:
	/** Handle seed response */
	void OnSeedResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle track selection response */
	void OnTrackSelectionResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle track sequence response */
	void OnTrackSequenceResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle shop items response */
	void OnShopItemsResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle boss rewards response */
	void OnBossRewardsResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle run submission response */
	void OnRunSubmitResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle HTTP error */
	void OnHttpError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody);

	/** Base URL for API */
	FString GetBaseUrl() const;

	/** HTTP Client instance (helper class) */
	UPROPERTY()
	class UHttpClient* HttpClient;

	/** Delegates */
	UPROPERTY()
	FOnSeedReceived OnSeedReceived;

	UPROPERTY()
	FOnError OnError;

	UPROPERTY()
	FOnTrackSelectionReceived OnTrackSelectionReceived;

	UPROPERTY()
	FOnTrackSequenceReceived OnTrackSequenceReceived;

	UPROPERTY()
	FOnShopItemsReceived OnShopItemsReceived;

	UPROPERTY()
	FOnBossRewardsReceived OnBossRewardsReceived;
};
