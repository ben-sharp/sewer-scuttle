// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayQueryHandler.h"
#include "BlueprintQueryHandler.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"

TSharedPtr<FJsonObject> FGameplayQueryHandler::GetCharacters(const FQueryOptions& Options)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> CharactersArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Query all blueprint assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		// Filter for character-related blueprints
		FString AssetName = AssetData.AssetName.ToString();
		FString AssetPath = AssetData.GetObjectPathString();

		// Check if this is a character blueprint
		if (AssetName.Contains(TEXT("Character"), ESearchCase::IgnoreCase) ||
			AssetPath.Contains(TEXT("/Characters/"), ESearchCase::IgnoreCase) ||
			AssetPath.Contains(TEXT("Character"), ESearchCase::IgnoreCase))
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
			if (Blueprint && IsCharacterBlueprint(Blueprint))
			{
				TSharedPtr<FJsonObject> CharacterJson = ExtractCharacterData(Blueprint, Options);
				if (CharacterJson)
				{
					CharactersArray.Add(MakeShared<FJsonValueObject>(CharacterJson));
				}
			}
		}
	}

	Result->SetArrayField(TEXT("characters"), CharactersArray);
	Result->SetNumberField(TEXT("count"), CharactersArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d character blueprints"), CharactersArray.Num());

	return Result;
}

TSharedPtr<FJsonObject> FGameplayQueryHandler::GetWeapons(const FQueryOptions& Options)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> WeaponsArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Query all blueprint assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		// Filter for weapon-related blueprints
		FString AssetName = AssetData.AssetName.ToString();
		FString AssetPath = AssetData.GetObjectPathString();

		// Check if this is a weapon blueprint
		if (AssetName.Contains(TEXT("Weapon"), ESearchCase::IgnoreCase) ||
			AssetName.Contains(TEXT("Pistol"), ESearchCase::IgnoreCase) ||
			AssetName.Contains(TEXT("Rifle"), ESearchCase::IgnoreCase) ||
			AssetName.Contains(TEXT("Grenade"), ESearchCase::IgnoreCase) ||
			AssetPath.Contains(TEXT("/Weapons/"), ESearchCase::IgnoreCase) ||
			AssetPath.Contains(TEXT("/Variant_Shooter/"), ESearchCase::IgnoreCase))
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
			if (Blueprint && IsWeaponBlueprint(Blueprint))
			{
				TSharedPtr<FJsonObject> WeaponJson = ExtractWeaponData(Blueprint, Options);
				if (WeaponJson)
				{
					WeaponsArray.Add(MakeShared<FJsonValueObject>(WeaponJson));
				}
			}
		}
	}

	Result->SetArrayField(TEXT("weapons"), WeaponsArray);
	Result->SetNumberField(TEXT("count"), WeaponsArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d weapon blueprints"), WeaponsArray.Num());

	return Result;
}

TSharedPtr<FJsonObject> FGameplayQueryHandler::GetAIConfigurations(const FQueryOptions& Options)
{
	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> AIArray;

	// Get asset registry
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Query all blueprint assets
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), AssetDataList, true);

	for (const FAssetData& AssetData : AssetDataList)
	{
		// Filter for AI-related blueprints
		FString AssetName = AssetData.AssetName.ToString();
		FString AssetPath = AssetData.GetObjectPathString();

		// Check if this is an AI blueprint
		if (AssetName.Contains(TEXT("AI"), ESearchCase::IgnoreCase) ||
			AssetName.Contains(TEXT("NPC"), ESearchCase::IgnoreCase) ||
			AssetName.Contains(TEXT("Spawner"), ESearchCase::IgnoreCase) ||
			AssetPath.Contains(TEXT("/AI/"), ESearchCase::IgnoreCase))
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
			if (Blueprint && IsAIBlueprint(Blueprint))
			{
				TSharedPtr<FJsonObject> AIJson = ExtractAIData(Blueprint, Options);
				if (AIJson)
				{
					AIArray.Add(MakeShared<FJsonValueObject>(AIJson));
				}
			}
		}
	}

	Result->SetArrayField(TEXT("ai_configurations"), AIArray);
	Result->SetNumberField(TEXT("count"), AIArray.Num());
	Result->SetBoolField(TEXT("success"), true);

	UE_LOG(LogTemp, Log, TEXT("RevoltPlugin: Found %d AI blueprints"), AIArray.Num());

	return Result;
}

bool FGameplayQueryHandler::IsCharacterBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return false;
	}

	// Check if it's derived from ACharacter
	UClass* ParentClass = Blueprint->GeneratedClass;
	while (ParentClass)
	{
		FString ClassName = ParentClass->GetName();
		if (ClassName.Contains(TEXT("Character")))
		{
			return true;
		}
		ParentClass = ParentClass->GetSuperClass();
	}

	return false;
}

bool FGameplayQueryHandler::IsWeaponBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return false;
	}

	// Check if it's derived from a weapon class
	UClass* ParentClass = Blueprint->GeneratedClass;
	while (ParentClass)
	{
		FString ClassName = ParentClass->GetName();
		if (ClassName.Contains(TEXT("Weapon")) || ClassName.Contains(TEXT("Projectile")) || ClassName.Contains(TEXT("Pickup")))
		{
			return true;
		}
		ParentClass = ParentClass->GetSuperClass();
	}

	return false;
}

bool FGameplayQueryHandler::IsAIBlueprint(UBlueprint* Blueprint)
{
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return false;
	}

	// Check if it's derived from AI classes
	UClass* ParentClass = Blueprint->GeneratedClass;
	while (ParentClass)
	{
		FString ClassName = ParentClass->GetName();
		if (ClassName.Contains(TEXT("AI")) || 
			ClassName.Contains(TEXT("NPC")) || 
			ClassName.Contains(TEXT("Spawner")) ||
			ClassName.Contains(TEXT("Controller")))
		{
			return true;
		}
		ParentClass = ParentClass->GetSuperClass();
	}

	return false;
}

TSharedPtr<FJsonObject> FGameplayQueryHandler::ExtractCharacterData(UBlueprint* Blueprint, const FQueryOptions& Options)
{
	// Use blueprint query handler for base extraction
	TSharedPtr<FJsonObject> CharacterJson = FBlueprintQueryHandler::ExtractBlueprintInfo(Blueprint, Options);
	
	if (!CharacterJson || !Blueprint->GeneratedClass)
	{
		return CharacterJson;
	}

	// Add character-specific data
	CharacterJson->SetStringField(TEXT("type"), TEXT("character"));

	// Extract gameplay properties if available
	if (Options.bIncludeDefaultValues)
	{
		TSharedPtr<FJsonObject> GameplayData = MakeShared<FJsonObject>();
		
		// Try to extract common character properties
		FString MaxHP = GetPropertyValue(Blueprint->GeneratedClass, TEXT("MaxHP"));
		if (!MaxHP.IsEmpty())
		{
			GameplayData->SetStringField(TEXT("max_hp"), MaxHP);
		}

		FString CurrentHP = GetPropertyValue(Blueprint->GeneratedClass, TEXT("CurrentHP"));
		if (!CurrentHP.IsEmpty())
		{
			GameplayData->SetStringField(TEXT("current_hp"), CurrentHP);
		}

		FString TeamByte = GetPropertyValue(Blueprint->GeneratedClass, TEXT("TeamByte"));
		if (!TeamByte.IsEmpty())
		{
			GameplayData->SetStringField(TEXT("team"), TeamByte);
		}

		CharacterJson->SetObjectField(TEXT("gameplay_data"), GameplayData);
	}

	return CharacterJson;
}

TSharedPtr<FJsonObject> FGameplayQueryHandler::ExtractWeaponData(UBlueprint* Blueprint, const FQueryOptions& Options)
{
	// Use blueprint query handler for base extraction
	TSharedPtr<FJsonObject> WeaponJson = FBlueprintQueryHandler::ExtractBlueprintInfo(Blueprint, Options);
	
	if (!WeaponJson || !Blueprint->GeneratedClass)
	{
		return WeaponJson;
	}

	// Add weapon-specific data
	WeaponJson->SetStringField(TEXT("type"), TEXT("weapon"));

	// Extract weapon properties if available
	if (Options.bIncludeDefaultValues)
	{
		TSharedPtr<FJsonObject> WeaponData = MakeShared<FJsonObject>();
		
		// Try to extract common weapon properties
		FString MagazineSize = GetPropertyValue(Blueprint->GeneratedClass, TEXT("MagazineSize"));
		if (!MagazineSize.IsEmpty())
		{
			WeaponData->SetStringField(TEXT("magazine_size"), MagazineSize);
		}

		FString AimVariance = GetPropertyValue(Blueprint->GeneratedClass, TEXT("AimVariance"));
		if (!AimVariance.IsEmpty())
		{
			WeaponData->SetStringField(TEXT("aim_variance"), AimVariance);
		}

		FString FiringRecoil = GetPropertyValue(Blueprint->GeneratedClass, TEXT("FiringRecoil"));
		if (!FiringRecoil.IsEmpty())
		{
			WeaponData->SetStringField(TEXT("firing_recoil"), FiringRecoil);
		}

		WeaponJson->SetObjectField(TEXT("weapon_data"), WeaponData);
	}

	return WeaponJson;
}

TSharedPtr<FJsonObject> FGameplayQueryHandler::ExtractAIData(UBlueprint* Blueprint, const FQueryOptions& Options)
{
	// Use blueprint query handler for base extraction
	TSharedPtr<FJsonObject> AIJson = FBlueprintQueryHandler::ExtractBlueprintInfo(Blueprint, Options);
	
	if (!AIJson || !Blueprint->GeneratedClass)
	{
		return AIJson;
	}

	// Add AI-specific data
	AIJson->SetStringField(TEXT("type"), TEXT("ai"));

	// Extract AI properties if available
	if (Options.bIncludeDefaultValues)
	{
		TSharedPtr<FJsonObject> AIData = MakeShared<FJsonObject>();
		
		// Try to extract common AI properties
		FString AimRange = GetPropertyValue(Blueprint->GeneratedClass, TEXT("AimRange"));
		if (!AimRange.IsEmpty())
		{
			AIData->SetStringField(TEXT("aim_range"), AimRange);
		}

		FString AimVarianceHalfAngle = GetPropertyValue(Blueprint->GeneratedClass, TEXT("AimVarianceHalfAngle"));
		if (!AimVarianceHalfAngle.IsEmpty())
		{
			AIData->SetStringField(TEXT("aim_variance"), AimVarianceHalfAngle);
		}

		AIJson->SetObjectField(TEXT("ai_data"), AIData);
	}

	return AIJson;
}

FString FGameplayQueryHandler::GetPropertyValue(UClass* Class, const FString& PropertyName)
{
	if (!Class)
	{
		return FString();
	}

	FProperty* Property = Class->FindPropertyByName(*PropertyName);
	if (!Property)
	{
		return FString();
	}

	UObject* CDO = Class->GetDefaultObject();
	if (!CDO)
	{
		return FString();
	}

	const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CDO);
	FString ValueString;
	Property->ExportTextItem_Direct(ValueString, ValuePtr, nullptr, nullptr, PPF_None);

	return ValueString;
}

