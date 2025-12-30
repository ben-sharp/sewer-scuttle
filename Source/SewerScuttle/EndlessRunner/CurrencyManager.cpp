// Copyright Epic Games, Inc. All Rights Reserved.

#include "CurrencyManager.h"
#include "WebServerInterface.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

UCurrencyManager::UCurrencyManager()
{
	Coins = 0;
	WebServerInterface = nullptr;
}

void UCurrencyManager::Initialize()
{
	// Create web server interface
	WebServerInterface = NewObject<UWebServerInterface>(this);

	// Load coins from server (stub for now - will load from server when implemented)
	LoadCoins();
}

void UCurrencyManager::AddCoins(int32 Amount)
{
	Coins += Amount;
	// Save to server (async)
	SaveCoins();
}

bool UCurrencyManager::SpendCoins(int32 Amount)
{
	if (Coins >= Amount)
	{
		Coins -= Amount;
		// Save to server (async)
		SaveCoins();
		return true;
	}
	return false;
}

void UCurrencyManager::SaveCoins()
{
	// Save locally as temporary cache
	SaveCoinsLocal();

	// Save to web server (async) - server is source of truth
	if (WebServerInterface)
	{
		WebServerInterface->SaveCurrency(Coins);
	}
}

void UCurrencyManager::LoadCoins()
{
	// Load from local cache first (for offline/quick access)
	LoadCoinsLocal();

	// Load from web server (async) - server is source of truth
	if (WebServerInterface)
	{
		WebServerInterface->LoadCurrency();
	}
}

bool UCurrencyManager::PurchaseItem(const FString& ItemId, int32 Price)
{
	if (SpendCoins(Price))
	{
		// Notify web server of purchase
		if (WebServerInterface)
		{
			WebServerInterface->PurchaseItem(ItemId, Price);
		}
		return true;
	}
	return false;
}

void UCurrencyManager::SaveCoinsLocal()
{
	// Temporary local cache (server is source of truth)
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Coins.dat");
	FString CoinsString = FString::Printf(TEXT("%d"), Coins);
	FFileHelper::SaveStringToFile(CoinsString, *SavePath);
}

void UCurrencyManager::LoadCoinsLocal()
{
	// Load from temporary local cache (server is source of truth)
	FString SavePath = FPaths::ProjectSavedDir() / TEXT("Coins.dat");
	FString CoinsString;
	
	if (FFileHelper::LoadFileToString(CoinsString, *SavePath))
	{
		Coins = FCString::Atoi(*CoinsString);
	}
	else
	{
		Coins = 0;
	}
}

