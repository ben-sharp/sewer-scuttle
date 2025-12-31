// Copyright Epic Games, Inc. All Rights Reserved.

#include "WebServerInterface.h"
#include "HttpClient.h"
#include "AuthService.h"
#include "DeviceIdManager.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Misc/DateTime.h"

UWebServerInterface::UWebServerInterface()
{
	HttpClient = nullptr;
	AuthService = nullptr;
}

void UWebServerInterface::Initialize()
{
	AuthService = UAuthService::Get();
	HttpClient = NewObject<UHttpClient>(this);
	HttpClient->Initialize(AuthService);
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Initialized"));
}

void UWebServerInterface::LoadCurrency()
{
	if (!HttpClient)
	{
		Initialize();
	}

	// Check if authenticated
	if (!AuthService || !AuthService->IsAuthenticated())
	{
		FString ErrorMsg = TEXT("Not authenticated. Please login first.");
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
		return;
	}

	HttpClient->Get(TEXT("/player/currency"),
		FOnHttpResponse::CreateUObject(this, &UWebServerInterface::OnCurrencyResponse),
		FOnHttpError::CreateUObject(this, &UWebServerInterface::OnHttpError));
}

void UWebServerInterface::PostScore(int32 Score, float Distance)
{
	if (!HttpClient)
	{
		Initialize();
	}

	// Check if authenticated
	if (!AuthService || !AuthService->IsAuthenticated())
	{
		FString ErrorMsg = TEXT("Not authenticated. Please login first.");
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
		return;
	}

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("score"), Score);
	JsonObject->SetNumberField(TEXT("distance"), Distance);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(TEXT("/leaderboard/submit"), RequestBody,
		FOnHttpResponse::CreateUObject(this, &UWebServerInterface::OnScorePostResponse),
		FOnHttpError::CreateUObject(this, &UWebServerInterface::OnHttpError));
}

void UWebServerInterface::GetLeaderboard(int32 TopCount)
{
	if (!HttpClient)
	{
		Initialize();
	}

	// Check if authenticated
	if (!AuthService || !AuthService->IsAuthenticated())
	{
		FString ErrorMsg = TEXT("Not authenticated. Please login first.");
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
		return;
	}

	FString Endpoint = FString::Printf(TEXT("/leaderboard?top=%d"), TopCount);
	HttpClient->Get(Endpoint,
		FOnHttpResponse::CreateUObject(this, &UWebServerInterface::OnLeaderboardResponse),
		FOnHttpError::CreateUObject(this, &UWebServerInterface::OnHttpError));
}

void UWebServerInterface::PurchaseItem(const FString& ItemId, int32 Price)
{
	if (!HttpClient)
	{
		Initialize();
	}

	// Check if authenticated
	if (!AuthService || !AuthService->IsAuthenticated())
	{
		FString ErrorMsg = TEXT("Not authenticated. Please login first.");
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
		return;
	}

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("item_id"), ItemId);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(TEXT("/store/purchase"), RequestBody,
		FOnHttpResponse::CreateUObject(this, &UWebServerInterface::OnPurchaseResponse),
		FOnHttpError::CreateUObject(this, &UWebServerInterface::OnHttpError));
}

void UWebServerInterface::OnCurrencyResponse(int32 ResponseCode, const FString& ResponseBody)
{
	TSharedPtr<FJsonValue> JsonValue;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (FJsonSerializer::Deserialize(Reader, JsonValue) && JsonValue.IsValid())
	{
		int32 Currency = 0;
		
		// Response is an array of currency objects
		if (JsonValue->Type == EJson::Array)
		{
			TArray<TSharedPtr<FJsonValue>> CurrencyArray = JsonValue->AsArray();
			for (const TSharedPtr<FJsonValue>& CurrencyValue : CurrencyArray)
			{
				if (CurrencyValue->Type == EJson::Object)
				{
					TSharedPtr<FJsonObject> CurrencyObj = CurrencyValue->AsObject();
					FString CurrencyType = CurrencyObj->GetStringField(TEXT("currency_type"));
					if (CurrencyType == TEXT("coins"))
					{
						Currency = CurrencyObj->GetIntegerField(TEXT("amount"));
						break; // Found coins, exit loop
					}
				}
			}
		}
		// Fallback: try parsing as object (for backwards compatibility)
		else if (JsonValue->Type == EJson::Object)
		{
			TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
			if (JsonObject->HasField(TEXT("coins")))
			{
				Currency = JsonObject->GetIntegerField(TEXT("coins"));
			}
		}

		if (OnCurrencyLoaded.IsBound())
		{
			OnCurrencyLoaded.Execute(Currency);
		}
	}
	else
	{
		FString ErrorMsg = TEXT("Failed to parse currency response");
		UE_LOG(LogTemp, Error, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
	}
}

void UWebServerInterface::OnLeaderboardResponse(int32 ResponseCode, const FString& ResponseBody)
{
	TArray<FString> Scores;

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		// Parse leaderboard response - adjust based on actual API response structure
		if (JsonObject->HasField(TEXT("data")))
		{
			TArray<TSharedPtr<FJsonValue>> DataArray = JsonObject->GetArrayField(TEXT("data"));
			for (const TSharedPtr<FJsonValue>& Value : DataArray)
			{
				if (Value->Type == EJson::Object)
				{
					TSharedPtr<FJsonObject> Entry = Value->AsObject();
					FString ScoreStr = FString::Printf(TEXT("%s: %d"), 
						*Entry->GetStringField(TEXT("display_name")),
						Entry->GetIntegerField(TEXT("score")));
					Scores.Add(ScoreStr);
				}
			}
		}
	}

	if (OnLeaderboardLoaded.IsBound())
	{
		OnLeaderboardLoaded.Execute(Scores);
	}
}

void UWebServerInterface::OnScorePostResponse(int32 ResponseCode, const FString& ResponseBody)
{
	bool bSuccess = ResponseCode >= 200 && ResponseCode < 300;
	if (OnScorePosted.IsBound())
	{
		OnScorePosted.Execute(bSuccess);
	}
}

void UWebServerInterface::OnPurchaseResponse(int32 ResponseCode, const FString& ResponseBody)
{
	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Purchase successful"));
	}
	else
	{
		FString ErrorMsg = TEXT("Purchase failed");
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s"), *ErrorMsg);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
	}
}

void UWebServerInterface::RequestRunSeed(int32 MaxDistance)
{
	if (!HttpClient)
	{
		Initialize();
	}

	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Requesting run seed (max_distance: %d)"), MaxDistance);

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	
	// Add device_id if available (for anonymous runs)
	UDeviceIdManager* DeviceIdManager = UDeviceIdManager::Get();
	if (DeviceIdManager && DeviceIdManager->HasDeviceId())
	{
		JsonObject->SetStringField(TEXT("device_id"), DeviceIdManager->GetDeviceId());
		UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Including device_id: %s"), *DeviceIdManager->GetDeviceId());
	}

	if (MaxDistance > 0)
	{
		JsonObject->SetNumberField(TEXT("max_distance"), MaxDistance);
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: POST /runs/start - Body: %s"), *RequestBody);

	HttpClient->Post(TEXT("/runs/start"), RequestBody,
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			OnSeedResponse(ResponseCode, ResponseBody);
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
		}));
}

void UWebServerInterface::SubmitRun(const FString& SeedId, int32 Score, int32 Distance, int32 DurationSeconds,
	int32 CoinsCollected, int32 ObstaclesHit, int32 PowerupsUsed, int32 TrackPiecesSpawned, const FString& StartedAt)
{
	if (!HttpClient)
	{
		Initialize();
	}

	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Submitting run (seed_id: %s, score: %d, distance: %d)"), 
		*SeedId, Score, Distance);

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("seed_id"), SeedId);
	JsonObject->SetNumberField(TEXT("score"), Score);
	JsonObject->SetNumberField(TEXT("distance"), Distance);
	JsonObject->SetNumberField(TEXT("duration_seconds"), DurationSeconds);
	JsonObject->SetNumberField(TEXT("coins_collected"), CoinsCollected);
	JsonObject->SetNumberField(TEXT("obstacles_hit"), ObstaclesHit);
	JsonObject->SetNumberField(TEXT("powerups_used"), PowerupsUsed);
	JsonObject->SetNumberField(TEXT("track_pieces_spawned"), TrackPiecesSpawned);
	JsonObject->SetStringField(TEXT("started_at"), StartedAt);

	// Add device_id if available (for anonymous runs)
	UDeviceIdManager* DeviceIdManager = UDeviceIdManager::Get();
	if (DeviceIdManager && DeviceIdManager->HasDeviceId())
	{
		JsonObject->SetStringField(TEXT("device_id"), DeviceIdManager->GetDeviceId());
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: POST /runs - Body: %s"), *RequestBody);

	HttpClient->Post(TEXT("/runs"), RequestBody,
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			OnRunSubmitResponse(ResponseCode, ResponseBody);
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
		}));
}

void UWebServerInterface::OnSeedResponse(int32 ResponseCode, const FString& ResponseBody)
{
	UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: Seed response received - Code: %d, Body: %s"), ResponseCode, *ResponseBody);

	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FRunSeedData SeedData;
			SeedData.SeedId = JsonObject->GetStringField(TEXT("seed_id"));
			SeedData.Seed = JsonObject->GetIntegerField(TEXT("seed"));
			SeedData.ContentVersion = JsonObject->GetStringField(TEXT("content_version"));
			SeedData.MaxCoins = JsonObject->GetIntegerField(TEXT("max_coins"));
			SeedData.MaxObstacles = JsonObject->GetIntegerField(TEXT("max_obstacles"));
			SeedData.MaxTrackPieces = JsonObject->GetIntegerField(TEXT("max_track_pieces"));
			SeedData.MaxDistance = JsonObject->GetIntegerField(TEXT("max_distance"));

			UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: Seed parsed successfully - Seed: %d, SeedId: %s, MaxCoins: %d, MaxObstacles: %d"),
				SeedData.Seed, *SeedData.SeedId, SeedData.MaxCoins, SeedData.MaxObstacles);

			if (OnSeedReceived.IsBound())
			{
				UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: Executing OnSeedReceived delegate..."));
				OnSeedReceived.Execute(SeedData);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WebServerInterface: OnSeedReceived delegate is NOT bound!"));
			}
		}
		else
		{
			FString ErrorMsg = TEXT("Failed to parse seed response");
			UE_LOG(LogTemp, Error, TEXT("WebServerInterface: %s - ResponseBody: %s"), *ErrorMsg, *ResponseBody);
			if (OnError.IsBound())
			{
				OnError.Execute(ErrorMsg);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("WebServerInterface: OnError delegate is NOT bound!"));
			}
		}
	}
	else
	{
		FString ErrorMsg = FString::Printf(TEXT("Failed to get seed (HTTP %d)"), ResponseCode);
		UE_LOG(LogTemp, Error, TEXT("WebServerInterface: %s - ResponseBody: %s"), *ErrorMsg, *ResponseBody);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("WebServerInterface: OnError delegate is NOT bound!"));
		}
	}
}

void UWebServerInterface::OnRunSubmitResponse(int32 ResponseCode, const FString& ResponseBody)
{
	bool bSuccess = ResponseCode >= 200 && ResponseCode < 300;
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Run submit response - Code: %d, Success: %d"), ResponseCode, bSuccess);
	
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Run submitted successfully"));
	}
	else
	{
		FString ErrorMsg = FString::Printf(TEXT("Failed to submit run (HTTP %d)"), ResponseCode);
		UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: %s - %s"), *ErrorMsg, *ResponseBody);
		if (OnError.IsBound())
		{
			OnError.Execute(ErrorMsg);
		}
	}
}

void UWebServerInterface::OnHttpError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
{
	FString FinalError = ErrorMessage;
	
	// Try to parse error from response
	if (!ResponseBody.IsEmpty())
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject->HasField(TEXT("message")))
			{
				FinalError = JsonObject->GetStringField(TEXT("message"));
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: HTTP Error %d - %s"), ResponseCode, *FinalError);
	if (OnError.IsBound())
	{
		OnError.Execute(FinalError);
	}
}

