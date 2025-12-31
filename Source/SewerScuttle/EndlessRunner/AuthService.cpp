// Copyright Epic Games, Inc. All Rights Reserved.

#include "AuthService.h"
#include "HttpClient.h"
#include "SecureStorage.h"
#include "DeviceIdManager.h"
#include "ConfigManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

UAuthService* UAuthService::Instance = nullptr;

UAuthService::UAuthService()
{
	HttpClient = nullptr;
	SecureStorage = nullptr;
	DeviceIdManager = nullptr;
	ConfigManager = nullptr;
	bIsAuthenticated = false;
	bIsRefreshingToken = false;
}

UAuthService* UAuthService::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UAuthService>();
		Instance->AddToRoot(); // Prevent garbage collection
		Instance->Initialize();
	}
	return Instance;
}

void UAuthService::Initialize()
{
	// Initialize dependencies
	ConfigManager = UConfigManager::Get();
	SecureStorage = NewObject<USecureStorage>(this);
	DeviceIdManager = UDeviceIdManager::Get();
	
	HttpClient = NewObject<UHttpClient>(this);
	HttpClient->Initialize(this);

	// Try to load stored token
	LoadStoredToken();

	UE_LOG(LogTemp, Log, TEXT("AuthService: Initialized"));
}

void UAuthService::LoadStoredToken()
{
	FString StoredToken;
	if (SecureStorage->LoadToken(StoredToken) && !StoredToken.IsEmpty())
	{
		AuthToken = StoredToken;
		HttpClient->SetAuthToken(AuthToken);
		
		// Try to get user data to verify token is still valid
		GetUserData();
	}
}

void UAuthService::SaveToken(const FString& Token)
{
	AuthToken = Token;
	HttpClient->SetAuthToken(AuthToken);
	SecureStorage->SaveToken(Token);
}

void UAuthService::ClearStoredToken()
{
	AuthToken.Empty();
	HttpClient->ClearAuthToken();
	SecureStorage->ClearToken();
}

void UAuthService::Login(const FString& Email, const FString& Password)
{
	if (Email.IsEmpty() || Password.IsEmpty())
	{
		OnLoginFailed.Broadcast(TEXT("Email and password are required"));
		return;
	}

	// Validate email format (simple check)
	if (!Email.Contains(TEXT("@")) || !Email.Contains(TEXT(".")))
	{
		OnLoginFailed.Broadcast(TEXT("Invalid email format"));
		return;
	}

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("email"), Email);
	JsonObject->SetStringField(TEXT("password"), Password);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Make request
	HttpClient->Post(TEXT("/auth/login"), RequestBody,
		FOnHttpResponse::CreateUObject(this, &UAuthService::OnLoginResponse),
		FOnHttpError::CreateUObject(this, &UAuthService::OnLoginError));
}

void UAuthService::Register(const FRegisterRequest& RegisterData)
{
	if (RegisterData.Email.IsEmpty() || RegisterData.Password.IsEmpty() || 
		RegisterData.Name.IsEmpty() || RegisterData.Username.IsEmpty() || 
		RegisterData.DisplayName.IsEmpty())
	{
		OnLoginFailed.Broadcast(TEXT("All fields are required"));
		return;
	}

	// Validate email format
	if (!RegisterData.Email.Contains(TEXT("@")) || !RegisterData.Email.Contains(TEXT(".")))
	{
		OnLoginFailed.Broadcast(TEXT("Invalid email format"));
		return;
	}

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("name"), RegisterData.Name);
	JsonObject->SetStringField(TEXT("email"), RegisterData.Email);
	JsonObject->SetStringField(TEXT("password"), RegisterData.Password);
	JsonObject->SetStringField(TEXT("username"), RegisterData.Username);
	JsonObject->SetStringField(TEXT("display_name"), RegisterData.DisplayName);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Make request
	HttpClient->Post(TEXT("/auth/register"), RequestBody,
		FOnHttpResponse::CreateUObject(this, &UAuthService::OnLoginResponse),
		FOnHttpError::CreateUObject(this, &UAuthService::OnLoginError));
}

void UAuthService::DeviceAuth(const FString& Username, const FString& DisplayName)
{
	if (Username.IsEmpty() || DisplayName.IsEmpty())
	{
		OnLoginFailed.Broadcast(TEXT("Username and display name are required"));
		return;
	}

	FString DeviceId = DeviceIdManager->GetDeviceId();
	if (DeviceId.IsEmpty())
	{
		OnLoginFailed.Broadcast(TEXT("Failed to get device ID"));
		return;
	}

	// Build request body
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("device_id"), DeviceId);
	JsonObject->SetStringField(TEXT("username"), Username);
	JsonObject->SetStringField(TEXT("display_name"), DisplayName);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	// Make request
	HttpClient->Post(TEXT("/auth/device"), RequestBody,
		FOnHttpResponse::CreateUObject(this, &UAuthService::OnLoginResponse),
		FOnHttpError::CreateUObject(this, &UAuthService::OnLoginError));
}

void UAuthService::Logout()
{
	if (!bIsAuthenticated)
	{
		return;
	}

	// Call logout endpoint
	HttpClient->Post(TEXT("/auth/logout"), TEXT(""),
		FOnHttpResponse::CreateLambda([this](int32 ResponseCode, const FString& ResponseBody)
		{
			// Clear local state regardless of response
			ClearStoredToken();
			SetAuthenticated(false);
			OnLogout.Broadcast();
			UE_LOG(LogTemp, Log, TEXT("AuthService: Logged out"));
		}),
		FOnHttpError::CreateLambda([this](int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
		{
			// Clear local state even on error
			ClearStoredToken();
			SetAuthenticated(false);
			OnLogout.Broadcast();
			UE_LOG(LogTemp, Warning, TEXT("AuthService: Logout error but cleared local state"));
		}));
}

void UAuthService::RefreshToken()
{
	if (bIsRefreshingToken || AuthToken.IsEmpty())
	{
		return;
	}

	bIsRefreshingToken = true;

	HttpClient->Post(TEXT("/auth/refresh"), TEXT(""),
		FOnHttpResponse::CreateUObject(this, &UAuthService::OnRefreshResponse),
		FOnHttpError::CreateUObject(this, &UAuthService::OnRefreshError));
}

void UAuthService::GetUserData()
{
	if (AuthToken.IsEmpty())
	{
		return;
	}

	HttpClient->Get(TEXT("/auth/user"),
		FOnHttpResponse::CreateUObject(this, &UAuthService::OnUserDataResponse),
		FOnHttpError::CreateUObject(this, &UAuthService::OnUserDataError));
}

bool UAuthService::HandleTokenExpired()
{
	if (bIsRefreshingToken)
	{
		return false; // Already refreshing
	}

	UE_LOG(LogTemp, Log, TEXT("AuthService: Handling token expiration"));
	RefreshToken();
	return true;
}

void UAuthService::SetAuthenticated(bool bAuthenticated, const FAuthUserData& UserData)
{
	bIsAuthenticated = bAuthenticated;
	if (bAuthenticated)
	{
		CurrentUser = UserData;
	}
	else
	{
		CurrentUser = FAuthUserData();
	}
	OnAuthStateChanged.Broadcast(bIsAuthenticated);
}

bool UAuthService::ParseAuthResponse(const FString& ResponseBody, FAuthResponse& OutResponse)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	// Parse token
	if (JsonObject->HasField(TEXT("token")))
	{
		OutResponse.Token = JsonObject->GetStringField(TEXT("token"));
	}

	// Parse user data
	if (JsonObject->HasField(TEXT("user")))
	{
		TSharedPtr<FJsonObject> UserObject = JsonObject->GetObjectField(TEXT("user"));
		if (UserObject.IsValid())
		{
			OutResponse.User.Id = UserObject->GetIntegerField(TEXT("id"));
			OutResponse.User.Name = UserObject->GetStringField(TEXT("name"));
			OutResponse.User.Email = UserObject->GetStringField(TEXT("email"));

			// Parse player data
			if (UserObject->HasField(TEXT("player")))
			{
				TSharedPtr<FJsonObject> PlayerObject = UserObject->GetObjectField(TEXT("player"));
				if (PlayerObject.IsValid())
				{
					OutResponse.User.Player.Id = PlayerObject->GetIntegerField(TEXT("id"));
					OutResponse.User.Player.Username = PlayerObject->GetStringField(TEXT("username"));
					OutResponse.User.Player.DisplayName = PlayerObject->GetStringField(TEXT("display_name"));

					// Parse currency
					if (PlayerObject->HasField(TEXT("currency")))
					{
						TSharedPtr<FJsonObject> CurrencyObject = PlayerObject->GetObjectField(TEXT("currency"));
						if (CurrencyObject.IsValid())
						{
							for (const auto& Pair : CurrencyObject->Values)
							{
								OutResponse.User.Player.Currency.Add(Pair.Key, Pair.Value->AsNumber());
							}
						}
					}
				}
			}
		}
	}

	return true;
}

bool UAuthService::ParseUserData(const FString& ResponseBody, FAuthUserData& OutUserData)
{
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	OutUserData.Id = JsonObject->GetIntegerField(TEXT("id"));
	OutUserData.Name = JsonObject->GetStringField(TEXT("name"));
	OutUserData.Email = JsonObject->GetStringField(TEXT("email"));

	// Parse player data
	if (JsonObject->HasField(TEXT("player")))
	{
		TSharedPtr<FJsonObject> PlayerObject = JsonObject->GetObjectField(TEXT("player"));
		if (PlayerObject.IsValid())
		{
			OutUserData.Player.Id = PlayerObject->GetIntegerField(TEXT("id"));
			OutUserData.Player.Username = PlayerObject->GetStringField(TEXT("username"));
			OutUserData.Player.DisplayName = PlayerObject->GetStringField(TEXT("display_name"));

			// Parse currency
			if (PlayerObject->HasField(TEXT("currency")))
			{
				TSharedPtr<FJsonObject> CurrencyObject = PlayerObject->GetObjectField(TEXT("currency"));
				if (CurrencyObject.IsValid())
				{
					for (const auto& Pair : CurrencyObject->Values)
					{
						OutUserData.Player.Currency.Add(Pair.Key, Pair.Value->AsNumber());
					}
				}
			}
		}
	}

	return true;
}

void UAuthService::OnLoginResponse(int32 ResponseCode, const FString& ResponseBody)
{
	FAuthResponse AuthResponse;
	if (ParseAuthResponse(ResponseBody, AuthResponse))
	{
		SaveToken(AuthResponse.Token);
		SetAuthenticated(true, AuthResponse.User);
		OnLoginSuccess.Broadcast(AuthResponse.User);
		UE_LOG(LogTemp, Log, TEXT("AuthService: Login successful"));
	}
	else
	{
		OnLoginFailed.Broadcast(TEXT("Failed to parse response"));
		UE_LOG(LogTemp, Error, TEXT("AuthService: Failed to parse login response"));
	}
}

void UAuthService::OnLoginError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
{
	FString FinalErrorMessage = ErrorMessage;
	
	// Try to parse error message from response
	if (!ResponseBody.IsEmpty())
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			if (JsonObject->HasField(TEXT("message")))
			{
				FinalErrorMessage = JsonObject->GetStringField(TEXT("message"));
			}
		}
	}

	OnLoginFailed.Broadcast(FinalErrorMessage);
	UE_LOG(LogTemp, Warning, TEXT("AuthService: Login failed - %s"), *FinalErrorMessage);
}

void UAuthService::OnRefreshResponse(int32 ResponseCode, const FString& ResponseBody)
{
	bIsRefreshingToken = false;

	FAuthResponse AuthResponse;
	if (ParseAuthResponse(ResponseBody, AuthResponse))
	{
		SaveToken(AuthResponse.Token);
		SetAuthenticated(true, AuthResponse.User);
		UE_LOG(LogTemp, Log, TEXT("AuthService: Token refreshed successfully"));
	}
	else
	{
		// Refresh failed, logout
		ClearStoredToken();
		SetAuthenticated(false);
		OnTokenExpired.Broadcast();
		UE_LOG(LogTemp, Warning, TEXT("AuthService: Token refresh failed"));
	}
}

void UAuthService::OnRefreshError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
{
	bIsRefreshingToken = false;

	// Refresh failed, logout
	ClearStoredToken();
	SetAuthenticated(false);
	OnTokenExpired.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("AuthService: Token refresh error - %s"), *ErrorMessage);
}

void UAuthService::OnUserDataResponse(int32 ResponseCode, const FString& ResponseBody)
{
	FAuthUserData UserData;
	if (ParseUserData(ResponseBody, UserData))
	{
		SetAuthenticated(true, UserData);
		OnUserDataLoaded.Broadcast(UserData);
		UE_LOG(LogTemp, Log, TEXT("AuthService: User data loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AuthService: Failed to parse user data"));
	}
}

void UAuthService::OnUserDataError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody)
{
	if (ResponseCode == 401)
	{
		// Token expired
		HandleTokenExpired();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuthService: Failed to load user data - %s"), *ErrorMessage);
	}
}

