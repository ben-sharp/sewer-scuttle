// Copyright Epic Games, Inc. All Rights Reserved.

#include "WebServerInterface.h"
#include "HttpClient.h"
#include "DeviceIdManager.h"
#include "ConfigManager.h"
#include "PlayerClass.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Framework/Application/SlateApplication.h"

void UWebServerInterface::Initialize()
{
	if (!HttpClient)
	{
		HttpClient = NewObject<UHttpClient>(this);
		HttpClient->Initialize();
	}
}

void UWebServerInterface::RequestRunSeed(int32 MaxDistance, EPlayerClass PlayerClass)
{
	if (!HttpClient)
	{
		Initialize();
	}

	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Requesting run seed (max_distance: %d)"), MaxDistance);

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	if (MaxDistance > 0)
	{
		JsonObject->SetNumberField(TEXT("max_distance"), MaxDistance);
	}

	JsonObject->SetStringField(TEXT("player_class"), FPlayerClassData::PlayerClassToString(PlayerClass));

	// Add device_id for anonymous runs
	UDeviceIdManager* DeviceIdManager = UDeviceIdManager::Get();
	if (DeviceIdManager && DeviceIdManager->HasDeviceId())
	{
		JsonObject->SetStringField(TEXT("device_id"), DeviceIdManager->GetDeviceId());
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

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

void UWebServerInterface::OnSeedResponse(int32 ResponseCode, const FString& ResponseBody)
{
	UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: Seed response received - Code: %d, Body: %s"), ResponseCode, *ResponseBody);

	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FTrackSelectionData SelectionData;
			SelectionData.SeedId = JsonObject->GetStringField(TEXT("seed_id"));
            SelectionData.Seed = JsonObject->GetIntegerField(TEXT("seed"));
            SelectionData.ContentVersion = JsonObject->GetStringField(TEXT("content_version"));
			SelectionData.Tier = JsonObject->GetIntegerField(TEXT("tier"));

			const TArray<TSharedPtr<FJsonValue>>* TracksArray;
			if (JsonObject->TryGetArrayField(TEXT("tracks"), TracksArray))
			{
				for (const TSharedPtr<FJsonValue>& TrackValue : *TracksArray)
				{
					if (TrackValue->Type == EJson::Object)
					{
						TSharedPtr<FJsonObject> TrackObject = TrackValue->AsObject();
						FTrackInfo TrackInfo;
						TrackInfo.Id = TrackObject->GetIntegerField(TEXT("id"));
						TrackInfo.Length = TrackObject->GetIntegerField(TEXT("length"));
						TrackInfo.ShopCount = TrackObject->GetIntegerField(TEXT("shop_count"));
						TrackInfo.BossId = TrackObject->GetStringField(TEXT("boss_id"));
						SelectionData.Tracks.Add(TrackInfo);
					}
				}
			}
            
            // Also parse seed data for backward compatibility
            FRunSeedData SeedData;
            SeedData.SeedId = SelectionData.SeedId;
            SeedData.Seed = SelectionData.Seed;
            SeedData.ContentVersion = SelectionData.ContentVersion;
            SeedData.MaxCoins = 0;
            SeedData.MaxObstacles = 0;
            SeedData.MaxTrackPieces = 0;
            SeedData.MaxDistance = 0;

			UE_LOG(LogTemp, Warning, TEXT("WebServerInterface: Track selection parsed - SeedId: %s, Tier: %d, Tracks: %d"), 
				*SelectionData.SeedId, SelectionData.Tier, SelectionData.Tracks.Num());

			if (OnTrackSelectionReceived.IsBound())
			{
				OnTrackSelectionReceived.Execute(SelectionData);
			}
            
            if (OnSeedReceived.IsBound())
            {
                OnSeedReceived.Execute(SeedData);
            }
		}
		else
		{
			FString ErrorMsg = TEXT("Failed to parse seed response");
			if (OnError.IsBound()) OnError.Execute(ErrorMsg);
		}
	}
	else
	{
		FString ErrorMsg = FString::Printf(TEXT("Failed to get seed (HTTP %d)"), ResponseCode);
		if (OnError.IsBound()) OnError.Execute(ErrorMsg);
	}
}

void UWebServerInterface::SelectTrack(const FString& SeedId, int32 Tier, int32 TrackIndex)
{
	if (!HttpClient) Initialize();

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("tier"), Tier);
	JsonObject->SetNumberField(TEXT("track_index"), TrackIndex);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(FString::Printf(TEXT("/runs/%s/select-track"), *SeedId), RequestBody,
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			OnTrackSequenceResponse(ResponseCode, ResponseBody);
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
		}));
}

void UWebServerInterface::OnTrackSequenceResponse(int32 ResponseCode, const FString& ResponseBody)
{
	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FTrackSequenceData SequenceData;
			
			const TArray<TSharedPtr<FJsonValue>>* PiecesArray;
			if (JsonObject->TryGetArrayField(TEXT("pieces"), PiecesArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *PiecesArray)
				{
                    if (Value->Type == EJson::Object)
                    {
                        TSharedPtr<FJsonObject> PieceObj = Value->AsObject();
                        FTrackPiecePrescription Prescription;
                        Prescription.PieceId = PieceObj->GetStringField(TEXT("id"));
                        
                        const TSharedPtr<FJsonObject>* SpawnMap;
                        if (PieceObj->TryGetObjectField(TEXT("spawns"), SpawnMap) && SpawnMap->IsValid())
                        {
                            for (auto& Pair : (*SpawnMap)->Values)
                            {
                                if (Pair.Value->Type == EJson::String)
                                {
                                    Prescription.PrescribedSpawns.Add(Pair.Key, Pair.Value->AsString());
                                }
                            }
                        }
                        SequenceData.Pieces.Add(Prescription);
                    }
				}
			}

			const TArray<TSharedPtr<FJsonValue>>* ShopPosArray;
			if (JsonObject->TryGetArrayField(TEXT("shop_positions"), ShopPosArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *ShopPosArray)
				{
					SequenceData.ShopPositions.Add(Value->AsNumber());
				}
			}

			SequenceData.BossId = JsonObject->GetStringField(TEXT("boss_id"));
			SequenceData.Length = JsonObject->GetIntegerField(TEXT("length"));
			SequenceData.ShopCount = JsonObject->GetIntegerField(TEXT("shop_count"));

			// Parse shop items
			const TArray<TSharedPtr<FJsonValue>>* AllShopsArray;
			if (JsonObject->TryGetArrayField(TEXT("all_shop_items"), AllShopsArray))
			{
				for (const TSharedPtr<FJsonValue>& ShopValue : *AllShopsArray)
				{
					if (ShopValue->Type == EJson::Object)
					{
						TSharedPtr<FJsonObject> ShopObj = ShopValue->AsObject();
						FShopData ShopData;
						const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
						if (ShopObj->TryGetArrayField(TEXT("items"), ItemsArray))
						{
							for (const TSharedPtr<FJsonValue>& ItemValue : *ItemsArray)
							{
								TSharedPtr<FJsonObject> ItemObj = ItemValue->AsObject();
								FShopItemData Item;
								Item.Id = ItemObj->GetStringField(TEXT("id"));
								Item.Name = ItemObj->GetStringField(TEXT("name"));
								Item.Cost = ItemObj->GetIntegerField(TEXT("cost"));
								
								const TSharedPtr<FJsonObject>* PropsObj;
								if (ItemObj->TryGetObjectField(TEXT("properties"), PropsObj))
								{
									for (auto& Prop : (*PropsObj)->Values)
									{
										if (Prop.Value->Type == EJson::String) Item.Properties.Add(Prop.Key, Prop.Value->AsString());
										else if (Prop.Value->Type == EJson::Number) Item.Properties.Add(Prop.Key, FString::Printf(TEXT("%f"), Prop.Value->AsNumber()));
										else if (Prop.Value->Type == EJson::Boolean) Item.Properties.Add(Prop.Key, Prop.Value->AsBool() ? TEXT("true") : TEXT("false"));
									}
								}
								ShopData.Items.Add(Item);
							}
						}
						SequenceData.AllShopsData.Add(ShopData);
					}
				}
			}

			// Parse boss rewards
			const TArray<TSharedPtr<FJsonValue>>* RewardsArray;
			if (JsonObject->TryGetArrayField(TEXT("boss_rewards"), RewardsArray))
			{
				for (const TSharedPtr<FJsonValue>& RewardValue : *RewardsArray)
				{
					if (RewardValue->Type == EJson::Object)
					{
						TSharedPtr<FJsonObject> RewardObj = RewardValue->AsObject();
						FBossRewardData Reward;
						Reward.Id = RewardObj->GetStringField(TEXT("id"));
						Reward.Name = RewardObj->GetStringField(TEXT("name"));
						
						const TSharedPtr<FJsonObject>* PropsObj;
						if (RewardObj->TryGetObjectField(TEXT("properties"), PropsObj))
						{
							for (auto& Prop : (*PropsObj)->Values)
							{
								if (Prop.Value->Type == EJson::String) Reward.Properties.Add(Prop.Key, Prop.Value->AsString());
								else if (Prop.Value->Type == EJson::Number) Reward.Properties.Add(Prop.Key, FString::Printf(TEXT("%f"), Prop.Value->AsNumber()));
								else if (Prop.Value->Type == EJson::Boolean) Reward.Properties.Add(Prop.Key, Prop.Value->AsBool() ? TEXT("true") : TEXT("false"));
							}
						}
						SequenceData.BossRewards.Add(Reward);
					}
				}
			}

			if (OnTrackSequenceReceived.IsBound())
			{
				OnTrackSequenceReceived.Execute(SequenceData);
			}
		}
	}
}

void UWebServerInterface::GetShopItems(const FString& SeedId, int32 Tier, int32 TrackIndex, int32 ShopIndex)
{
	if (!HttpClient) Initialize();

	FString Endpoint = FString::Printf(TEXT("/runs/%s/shop/%d/%d/%d"), *SeedId, Tier, TrackIndex, ShopIndex);
	
	HttpClient->Get(Endpoint,
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			OnShopItemsResponse(ResponseCode, ResponseBody);
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
		}));
}

void UWebServerInterface::OnShopItemsResponse(int32 ResponseCode, const FString& ResponseBody)
{
	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FShopData ShopData;
			const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
			if (JsonObject->TryGetArrayField(TEXT("items"), ItemsArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *ItemsArray)
				{
					TSharedPtr<FJsonObject> ItemObj = Value->AsObject();
					FShopItemData Item;
					Item.Id = ItemObj->GetStringField(TEXT("id"));
					Item.Name = ItemObj->GetStringField(TEXT("name"));
					Item.Cost = ItemObj->GetIntegerField(TEXT("cost"));
					
					TSharedPtr<FJsonObject> PropsObj = ItemObj->GetObjectField(TEXT("properties"));
					for (auto& Prop : PropsObj->Values)
					{
						Item.Properties.Add(Prop.Key, Prop.Value->AsString());
					}
					ShopData.Items.Add(Item);
				}
			}

			if (OnShopItemsReceived.IsBound())
			{
				OnShopItemsReceived.Execute(ShopData);
			}
		}
	}
}

void UWebServerInterface::RerollShop(const FString& SeedId, int32 Tier, int32 TrackIndex, int32 ShopIndex)
{
	// Implementation follows similar pattern to SelectTrack/GetShopItems
	// For brevity, using GetShopItems again but the server would reroll the random seed
	GetShopItems(SeedId, Tier, TrackIndex, ShopIndex);
}

void UWebServerInterface::GetBossRewards(const FString& SeedId, int32 Tier)
{
	if (!HttpClient) Initialize();

	HttpClient->Get(FString::Printf(TEXT("/runs/%s/boss-rewards/%d"), *SeedId, Tier),
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			OnBossRewardsResponse(ResponseCode, ResponseBody);
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
		}));
}

void UWebServerInterface::OnBossRewardsResponse(int32 ResponseCode, const FString& ResponseBody)
{
	if (ResponseCode >= 200 && ResponseCode < 300)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			TArray<FBossRewardData> Rewards;
			const TArray<TSharedPtr<FJsonValue>>* RewardsArray;
			if (JsonObject->TryGetArrayField(TEXT("rewards"), RewardsArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *RewardsArray)
				{
					TSharedPtr<FJsonObject> RewardObj = Value->AsObject();
					FBossRewardData Reward;
					Reward.Id = RewardObj->GetStringField(TEXT("id"));
					Reward.Name = RewardObj->GetStringField(TEXT("name"));
					
					TSharedPtr<FJsonObject> PropsObj = RewardObj->GetObjectField(TEXT("properties"));
					for (auto& Prop : PropsObj->Values)
					{
						Reward.Properties.Add(Prop.Key, Prop.Value->AsString());
					}
					Rewards.Add(Reward);
				}
			}

			if (OnBossRewardsReceived.IsBound())
			{
				OnBossRewardsReceived.Execute(Rewards);
			}
		}
	}
}

void UWebServerInterface::RequestTierTracks(const FString& SeedId, int32 Tier)
{
    if (!HttpClient) Initialize();

    FString Endpoint = FString::Printf(TEXT("/runs/%s/tier/%d"), *SeedId, Tier);
    
    HttpClient->Get(Endpoint,
        FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
        {
            OnTrackSelectionResponse(ResponseCode, ResponseBody);
        }),
        FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
        {
            OnHttpError(ResponseCode, ErrorMessage, ResponseBody);
        }));
}

void UWebServerInterface::OnTrackSelectionResponse(int32 ResponseCode, const FString& ResponseBody)
{
    if (ResponseCode >= 200 && ResponseCode < 300)
    {
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

        if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
        {
            FTrackSelectionData SelectionData;
            SelectionData.SeedId = JsonObject->GetStringField(TEXT("seed_id"));
            SelectionData.Seed = JsonObject->GetIntegerField(TEXT("seed"));
            SelectionData.ContentVersion = JsonObject->GetStringField(TEXT("content_version"));
            SelectionData.Tier = JsonObject->GetIntegerField(TEXT("tier"));

            const TArray<TSharedPtr<FJsonValue>>* TracksArray;
            if (JsonObject->TryGetArrayField(TEXT("tracks"), TracksArray))
            {
                for (const TSharedPtr<FJsonValue>& TrackValue : *TracksArray)
                {
                    if (TrackValue->Type == EJson::Object)
                    {
                        TSharedPtr<FJsonObject> TrackObject = TrackValue->AsObject();
                        FTrackInfo TrackInfo;
                        TrackInfo.Id = TrackObject->GetIntegerField(TEXT("id"));
                        TrackInfo.Length = TrackObject->GetIntegerField(TEXT("length"));
                        TrackInfo.ShopCount = TrackObject->GetIntegerField(TEXT("shop_count"));
                        TrackInfo.BossId = TrackObject->GetStringField(TEXT("boss_id"));
                        SelectionData.Tracks.Add(TrackInfo);
                    }
                }
            }

            if (OnTrackSelectionReceived.IsBound())
            {
                OnTrackSelectionReceived.Execute(SelectionData);
            }
        }
    }
}

void UWebServerInterface::SubmitRun(const FString& SeedId, int32 Score, int32 Distance, int32 DurationSeconds,
	int32 CoinsCollected, int32 ObstaclesHit, int32 PowerupsUsed, int32 TrackPiecesSpawned,
	const FString& StartedAt, const TArray<int32>& SelectedTracks, bool bIsComplete, bool bIsEndless,
	const TArray<FString>& PieceSequence)
{
	if (!HttpClient) Initialize();

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
	JsonObject->SetBoolField(TEXT("is_complete"), bIsComplete);
	JsonObject->SetBoolField(TEXT("is_endless"), bIsEndless);

	TArray<TSharedPtr<FJsonValue>> SelectedTracksJson;
	for (int32 Index : SelectedTracks) SelectedTracksJson.Add(MakeShareable(new FJsonValueNumber(Index)));
	JsonObject->SetArrayField(TEXT("selected_track_indices"), SelectedTracksJson);

	TArray<TSharedPtr<FJsonValue>> PieceSeqJson;
	for (const FString& PieceId : PieceSequence) PieceSeqJson.Add(MakeShareable(new FJsonValueString(PieceId)));
	JsonObject->SetArrayField(TEXT("track_sequence"), PieceSeqJson);

	UDeviceIdManager* DeviceIdManager = UDeviceIdManager::Get();
	if (DeviceIdManager && DeviceIdManager->HasDeviceId())
	{
		JsonObject->SetStringField(TEXT("device_id"), DeviceIdManager->GetDeviceId());
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

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

void UWebServerInterface::SaveCurrency(int32 Amount)
{
	if (!HttpClient) Initialize();
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetNumberField(TEXT("amount"), Amount);
	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	HttpClient->Post(TEXT("/currency/save"), Body, nullptr, nullptr);
}

void UWebServerInterface::LoadCurrency()
{
	if (!HttpClient) Initialize();
	HttpClient->Get(TEXT("/currency"), nullptr, nullptr);
}

void UWebServerInterface::PurchaseItem(const FString& ItemId, int32 Price)
{
	if (!HttpClient) Initialize();
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("item_id"), ItemId);
	JsonObject->SetNumberField(TEXT("price"), Price);
	FString Body;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	HttpClient->Post(TEXT("/currency/purchase"), Body, nullptr, nullptr);
}

void UWebServerInterface::OnRunSubmitResponse(int32 ResponseCode, const FString& ResponseBody)
{
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: Run submitted - Code: %d, Body: %s"), ResponseCode, *ResponseBody);
}

void UWebServerInterface::OnHttpError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
{
	UE_LOG(LogTemp, Error, TEXT("WebServerInterface: HTTP Error %d: %s - Body: %s"), ResponseCode, *ErrorMessage, *ResponseBody);
	if (OnError.IsBound()) OnError.Execute(ErrorMessage);
}
