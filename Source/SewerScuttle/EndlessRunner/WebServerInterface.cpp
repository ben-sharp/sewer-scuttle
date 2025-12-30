// Copyright Epic Games, Inc. All Rights Reserved.

#include "WebServerInterface.h"
#include "Engine/Engine.h"

UWebServerInterface::UWebServerInterface()
{
	BaseURL = TEXT("http://localhost:3000/api");
}

void UWebServerInterface::SaveCurrency(int32 Currency)
{
	// Stubbed implementation - ready for HTTP module integration
	FString Endpoint = TEXT("/currency");
	FString Body = FString::Printf(TEXT("{\"currency\":%d}"), Currency);
	
	// TODO: Implement HTTP POST request
	// MakeHTTPRequest(Endpoint, TEXT("POST"), Body);
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: SaveCurrency called with %d (stubbed)"), Currency);
}

void UWebServerInterface::LoadCurrency()
{
	// Stubbed implementation - ready for HTTP module integration
	FString Endpoint = TEXT("/currency");
	
	// TODO: Implement HTTP GET request
	// MakeHTTPRequest(Endpoint, TEXT("GET"));
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: LoadCurrency called (stubbed)"));
	
	// For now, call callback with 0 (no currency loaded)
	if (OnCurrencyLoaded.IsBound())
	{
		OnCurrencyLoaded.Execute(0);
	}
}

void UWebServerInterface::PostScore(int32 Score, float Distance)
{
	// Stubbed implementation - ready for HTTP module integration
	FString Endpoint = TEXT("/leaderboard");
	FString Body = FString::Printf(TEXT("{\"score\":%d,\"distance\":%.2f}"), Score, Distance);
	
	// TODO: Implement HTTP POST request
	// MakeHTTPRequest(Endpoint, TEXT("POST"), Body);
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: PostScore called with Score=%d, Distance=%.2f (stubbed)"), Score, Distance);
	
	// For now, call callback with success
	if (OnScorePosted.IsBound())
	{
		OnScorePosted.Execute(true);
	}
}

void UWebServerInterface::GetLeaderboard(int32 TopCount)
{
	// Stubbed implementation - ready for HTTP module integration
	FString Endpoint = FString::Printf(TEXT("/leaderboard?top=%d"), TopCount);
	
	// TODO: Implement HTTP GET request
	// MakeHTTPRequest(Endpoint, TEXT("GET"));
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: GetLeaderboard called with TopCount=%d (stubbed)"), TopCount);
	
	// For now, return empty leaderboard
	TArray<FString> EmptyScores;
	if (OnLeaderboardLoaded.IsBound())
	{
		OnLeaderboardLoaded.Execute(EmptyScores);
	}
}

void UWebServerInterface::PurchaseItem(const FString& ItemId, int32 Price)
{
	// Stubbed implementation - ready for HTTP module integration
	FString Endpoint = TEXT("/purchase");
	FString Body = FString::Printf(TEXT("{\"itemId\":\"%s\",\"price\":%d}"), *ItemId, Price);
	
	// TODO: Implement HTTP POST request
	// MakeHTTPRequest(Endpoint, TEXT("POST"), Body);
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: PurchaseItem called with ItemId=%s, Price=%d (stubbed)"), *ItemId, Price);
}

void UWebServerInterface::MakeHTTPRequest(const FString& Endpoint, const FString& Method, const FString& Body)
{
	// Stubbed - ready for HTTP module integration
	// This will use Unreal's HTTP module to make requests
	// Example:
	// TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	// Request->SetURL(BaseURL + Endpoint);
	// Request->SetVerb(Method);
	// Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	// if (!Body.IsEmpty())
	// {
	//     Request->SetContentAsString(Body);
	// }
	// Request->OnProcessRequestComplete().BindUObject(this, &UWebServerInterface::HandleHTTPResponse);
	// Request->ProcessRequest();
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: MakeHTTPRequest called - %s %s%s (stubbed)"), *Method, *BaseURL, *Endpoint);
}

void UWebServerInterface::HandleHTTPResponse(const FString& Response, const FString& Endpoint)
{
	// Stubbed - ready for JSON parsing and callback execution
	// This will parse the JSON response and call appropriate callbacks
	// Example:
	// TSharedPtr<FJsonObject> JsonObject;
	// TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response);
	// if (FJsonSerializer::Deserialize(Reader, JsonObject))
	// {
	//     // Parse and handle response
	// }
	
	UE_LOG(LogTemp, Log, TEXT("WebServerInterface: HandleHTTPResponse called for %s (stubbed)"), *Endpoint);
}

