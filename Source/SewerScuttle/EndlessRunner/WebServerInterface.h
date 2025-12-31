// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebServerInterface.generated.h"

class UHttpClient;
class UAuthService;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnCurrencyLoaded, int32, Currency);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLeaderboardLoaded, TArray<FString>, Scores);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnScorePosted, bool, Success);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnError, const FString&, ErrorMessage);

USTRUCT(BlueprintType)
struct FRunSeedData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString SeedId;

	UPROPERTY(BlueprintReadWrite)
	int32 Seed;

	UPROPERTY(BlueprintReadWrite)
	FString ContentVersion;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxCoins;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxObstacles;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxTrackPieces;

	UPROPERTY(BlueprintReadWrite)
	int32 MaxDistance;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSeedReceived, const FRunSeedData&, SeedData);

/**
 * Interface for web server communication
 * Uses HttpClient with authentication support
 */
UCLASS()
class SEWERSCUTTLE_API UWebServerInterface : public UObject
{
	GENERATED_BODY()

public:
	UWebServerInterface();

	/** Initialize web server interface */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void Initialize();

	/** Load currency from web server */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void LoadCurrency();

	/** Post score to leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void PostScore(int32 Score, float Distance);

	/** Get leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void GetLeaderboard(int32 TopCount = 10);

	/** Purchase item */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void PurchaseItem(const FString& ItemId, int32 Price);

	/** Request seed for new run */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void RequestRunSeed(int32 MaxDistance = 0);

	/** Submit completed run */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SubmitRun(const FString& SeedId, int32 Score, int32 Distance, int32 DurationSeconds, 
		int32 CoinsCollected, int32 ObstaclesHit, int32 PowerupsUsed, int32 TrackPiecesSpawned,
		const FString& StartedAt);

	/** Set callback for currency loaded */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnCurrencyLoaded(FOnCurrencyLoaded Callback) { OnCurrencyLoaded = Callback; }

	/** Set callback for leaderboard loaded */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnLeaderboardLoaded(FOnLeaderboardLoaded Callback) { OnLeaderboardLoaded = Callback; }

	/** Set callback for score posted */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnScorePosted(FOnScorePosted Callback) { OnScorePosted = Callback; }

	/** Set callback for errors */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnError(FOnError Callback) { OnError = Callback; }

	/** Set callback for seed received */
	UFUNCTION(BlueprintCallable, Category = "Web Server")
	void SetOnSeedReceived(FOnSeedReceived Callback) { OnSeedReceived = Callback; }

protected:
	/** Callback for currency loaded */
	UPROPERTY()
	FOnCurrencyLoaded OnCurrencyLoaded;

	/** Callback for leaderboard loaded */
	UPROPERTY()
	FOnLeaderboardLoaded OnLeaderboardLoaded;

	/** Callback for score posted */
	UPROPERTY()
	FOnScorePosted OnScorePosted;

	/** Callback for errors */
	UPROPERTY()
	FOnError OnError;

	/** Callback for seed received */
	UPROPERTY()
	FOnSeedReceived OnSeedReceived;

	/** HTTP client */
	UPROPERTY()
	UHttpClient* HttpClient;

	/** Auth service */
	UPROPERTY()
	UAuthService* AuthService;

	/** Handle currency response */
	void OnCurrencyResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle leaderboard response */
	void OnLeaderboardResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle score post response */
	void OnScorePostResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle purchase response */
	void OnPurchaseResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle seed response */
	void OnSeedResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle run submit response */
	void OnRunSubmitResponse(int32 ResponseCode, const FString& ResponseBody);

	/** Handle HTTP error */
	void OnHttpError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody);
};

