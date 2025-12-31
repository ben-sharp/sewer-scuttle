// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuthModels.h"
#include "AuthService.generated.h"

class UHttpClient;
class USecureStorage;
class UDeviceIdManager;
class UConfigManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAuthStateChanged, bool, bIsAuthenticated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginSuccess, const FAuthUserData&, UserData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLogout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTokenExpired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUserDataLoaded, const FAuthUserData&, UserData);

/**
 * Authentication service managing all authentication operations
 * Singleton pattern for global access
 */
UCLASS(BlueprintType)
class SEWERSCUTTLE_API UAuthService : public UObject
{
	GENERATED_BODY()

public:
	UAuthService();

	/** Get singleton instance */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	static UAuthService* Get();

	/** Initialize authentication service */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void Initialize();

	/** Login with email and password */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void Login(const FString& Email, const FString& Password);

	/** Register new user */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void Register(const FRegisterRequest& RegisterData);

	/** Device authentication (anonymous) */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void DeviceAuth(const FString& Username, const FString& DisplayName);

	/** Logout */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void Logout();

	/** Refresh authentication token */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void RefreshToken();

	/** Get current user data */
	UFUNCTION(BlueprintCallable, Category = "Authentication")
	void GetUserData();

	/** Check if user is authenticated */
	UFUNCTION(BlueprintPure, Category = "Authentication")
	bool IsAuthenticated() const { return bIsAuthenticated; }

	/** Get current user data */
	UFUNCTION(BlueprintPure, Category = "Authentication")
	FAuthUserData GetCurrentUser() const { return CurrentUser; }

	/** Get authentication token */
	FString GetAuthToken() const { return AuthToken; }

	/** Check if token needs refresh (called by HttpClient on 401) */
	bool HandleTokenExpired();

	/** Delegates */
	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnAuthStateChanged OnAuthStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnLoginSuccess OnLoginSuccess;

	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnLoginFailed OnLoginFailed;

	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnLogout OnLogout;

	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnTokenExpired OnTokenExpired;

	UPROPERTY(BlueprintAssignable, Category = "Authentication")
	FOnUserDataLoaded OnUserDataLoaded;

private:
	/** Load token from secure storage */
	void LoadStoredToken();

	/** Save token to secure storage */
	void SaveToken(const FString& Token);

	/** Clear stored token */
	void ClearStoredToken();

	/** Parse auth response */
	bool ParseAuthResponse(const FString& ResponseBody, FAuthResponse& OutResponse);

	/** Parse user data */
	bool ParseUserData(const FString& ResponseBody, FAuthUserData& OutUserData);

	/** Set authenticated state */
	void SetAuthenticated(bool bAuthenticated, const FAuthUserData& UserData = FAuthUserData());

	/** HTTP response callbacks */
	UFUNCTION()
	void OnLoginResponse(int32 ResponseCode, const FString& ResponseBody);
	UFUNCTION()
	void OnLoginError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody);
	UFUNCTION()
	void OnRefreshResponse(int32 ResponseCode, const FString& ResponseBody);
	UFUNCTION()
	void OnRefreshError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody);
	UFUNCTION()
	void OnUserDataResponse(int32 ResponseCode, const FString& ResponseBody);
	UFUNCTION()
	void OnUserDataError(int32 ResponseCode, const FString& ErrorMessage, const FString& ResponseBody);

	/** HTTP client */
	UPROPERTY()
	UHttpClient* HttpClient;

	/** Secure storage */
	UPROPERTY()
	USecureStorage* SecureStorage;

	/** Device ID manager */
	UPROPERTY()
	UDeviceIdManager* DeviceIdManager;

	/** Config manager */
	UPROPERTY()
	UConfigManager* ConfigManager;

	/** Authentication token */
	FString AuthToken;

	/** Current user data */
	UPROPERTY()
	FAuthUserData CurrentUser;

	/** Is authenticated */
	UPROPERTY()
	bool bIsAuthenticated;

	/** Is refreshing token */
	bool bIsRefreshingToken;

	/** Singleton instance */
	static UAuthService* Instance;
};

