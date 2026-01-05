// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthService.h"
#include "HttpClient.h"
#include "SecureStorage.h"
#include "DeviceIdManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UAuthService::Login(const FUserCredentials& Credentials, FOnAuthSuccess OnSuccess, FOnAuthError OnError)
{
	if (!HttpClient)
	{
		HttpClient = NewObject<UHttpClient>(this);
		HttpClient->Initialize();
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("email"), Credentials.Email);
	JsonObject->SetStringField(TEXT("password"), Credentials.Password);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(TEXT("/login"), RequestBody,
		FOnHttpResponse::CreateLambda([this, OnSuccess, OnError](int32 ResponseCode, const FString& ResponseBody)
		{
			if (ResponseCode == 200)
			{
				TSharedPtr<FJsonObject> JsonResponse;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

				if (FJsonSerializer::Deserialize(Reader, JsonResponse))
				{
					FAuthResponse Response;
					Response.Token = JsonResponse->GetStringField(TEXT("token"));
					
					TSharedPtr<FJsonObject> UserObj = JsonResponse->GetObjectField(TEXT("user"));
					Response.User.Id = UserObj->GetIntegerField(TEXT("id"));
					Response.User.Name = UserObj->GetStringField(TEXT("name"));
					Response.User.Email = UserObj->GetStringField(TEXT("email"));

					AuthToken = Response.Token;
					CurrentUser = Response.User;

					// Persist token
					USecureStorage::Get()->Save(TEXT("auth_token"), AuthToken);

					OnSuccess.ExecuteIfBound(Response);
				}
				else
				{
					OnError.ExecuteIfBound(TEXT("Failed to parse response"));
				}
			}
			else
			{
				OnError.ExecuteIfBound(FString::Printf(TEXT("Login failed with code %d"), ResponseCode));
			}
		}),
		FOnHttpError::CreateLambda([OnError](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnError.ExecuteIfBound(ErrorMessage);
		}));
}

void UAuthService::Register(const FString& Name, const FString& Email, const FString& Password, FOnAuthSuccess OnSuccess, FOnAuthError OnError)
{
	if (!HttpClient)
	{
		HttpClient = NewObject<UHttpClient>(this);
		HttpClient->Initialize();
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), Name);
	JsonObject->SetStringField(TEXT("email"), Email);
	JsonObject->SetStringField(TEXT("password"), Password);
	
	// Add required profile fields for our backend
	JsonObject->SetStringField(TEXT("username"), Name.Replace(TEXT(" "), TEXT("_")).ToLower());
	JsonObject->SetStringField(TEXT("display_name"), Name);

	// Add device_id to link anonymous progress
	UDeviceIdManager* DeviceIdManager = UDeviceIdManager::Get();
	if (DeviceIdManager && DeviceIdManager->HasDeviceId())
	{
		JsonObject->SetStringField(TEXT("device_id"), DeviceIdManager->GetDeviceId());
	}

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	HttpClient->Post(TEXT("/register"), RequestBody,
		FOnHttpResponse::CreateLambda([this, OnSuccess, OnError](int32 ResponseCode, const FString& ResponseBody)
		{
			if (ResponseCode == 201)
			{
				TSharedPtr<FJsonObject> JsonResponse;
				TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

				if (FJsonSerializer::Deserialize(Reader, JsonResponse))
				{
					FAuthResponse Response;
					Response.Token = JsonResponse->GetStringField(TEXT("token"));
					
					TSharedPtr<FJsonObject> UserObj = JsonResponse->GetObjectField(TEXT("user"));
					Response.User.Id = UserObj->GetIntegerField(TEXT("id"));
					Response.User.Name = UserObj->GetStringField(TEXT("name"));
					Response.User.Email = UserObj->GetStringField(TEXT("email"));

					AuthToken = Response.Token;
					CurrentUser = Response.User;

					// Persist token
					USecureStorage::Get()->Save(TEXT("auth_token"), AuthToken);

					OnSuccess.ExecuteIfBound(Response);
				}
				else
				{
					OnError.ExecuteIfBound(TEXT("Failed to parse response"));
				}
			}
			else
			{
				OnError.ExecuteIfBound(FString::Printf(TEXT("Registration failed with code %d"), ResponseCode));
			}
		}),
		FOnHttpError::CreateLambda([OnError](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			OnError.ExecuteIfBound(ErrorMessage);
		}));
}

void UAuthService::RestoreSession()
{
	FString SavedToken = USecureStorage::Get()->Load(TEXT("auth_token"));
	if (!SavedToken.IsEmpty())
	{
		AuthToken = SavedToken;
		
		// Optionally verify token with /me endpoint
		if (!HttpClient)
		{
			HttpClient = NewObject<UHttpClient>(this);
			HttpClient->Initialize();
		}

		// We don't have a callback here for simplicity, 
		// but the token is now set for future HttpClient requests
	}
}

void UAuthService::Logout()
{
    AuthToken = TEXT("");
    USecureStorage::Get()->Remove(TEXT("auth_token"));
}

bool UAuthService::IsAuthenticated() const
{
    return !AuthToken.IsEmpty();
}

FString UAuthService::GetToken() const
{
    return AuthToken;
}

