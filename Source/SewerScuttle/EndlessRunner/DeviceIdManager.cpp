// Copyright Epic Games, Inc. All Rights Reserved.

#include "DeviceIdManager.h"
#include "SecureStorage.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif

UDeviceIdManager* UDeviceIdManager::Instance = nullptr;

UDeviceIdManager::UDeviceIdManager()
{
	SecureStorage = nullptr;
}

UDeviceIdManager* UDeviceIdManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UDeviceIdManager>();
		Instance->AddToRoot(); // Prevent garbage collection
		Instance->Initialize();
	}
	return Instance;
}

void UDeviceIdManager::Initialize()
{
	if (!SecureStorage)
	{
		SecureStorage = NewObject<USecureStorage>(this);
	}

	// Try to load existing device ID
	FString LoadedDeviceId;
	if (SecureStorage->LoadDeviceId(LoadedDeviceId) && !LoadedDeviceId.IsEmpty())
	{
		DeviceId = LoadedDeviceId;
		UE_LOG(LogTemp, Log, TEXT("DeviceIdManager: Loaded existing device ID"));
	}
	else
	{
		// Generate new device ID
		DeviceId = GenerateDeviceId();
		if (!DeviceId.IsEmpty())
		{
			SecureStorage->SaveDeviceId(DeviceId);
			UE_LOG(LogTemp, Log, TEXT("DeviceIdManager: Generated new device ID: %s"), *DeviceId);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DeviceIdManager: Failed to generate device ID"));
		}
	}
}

FString UDeviceIdManager::GenerateDeviceId()
{
#if PLATFORM_WINDOWS
	return GenerateWindowsDeviceId();
#elif PLATFORM_ANDROID
	return GenerateAndroidDeviceId();
#else
	return GenerateFallbackUUID();
#endif
}

FString UDeviceIdManager::GenerateWindowsDeviceId()
{
	FString ResultId;

#if PLATFORM_WINDOWS
	// Try to get hardware-based identifiers
	FString CpuId = FPlatformMisc::GetCPUBrand();
	FString MachineGuidStr = FPlatformMisc::GetLoginId();
	
	// Combine hardware identifiers
	FString HardwareString = CpuId + MachineGuidStr + FPlatformMisc::GetOSVersion();
	
	if (!HardwareString.IsEmpty())
	{
		// Create a hash from hardware string
		uint32 Hash = GetTypeHash(HardwareString);
		ResultId = FString::Printf(TEXT("WIN-%08X"), Hash);
	}
	else
	{
		ResultId = GenerateFallbackUUID();
	}
#else
	ResultId = GenerateFallbackUUID();
#endif

	return ResultId;
}

FString UDeviceIdManager::GenerateAndroidDeviceId()
{
	FString ResultId;

#if PLATFORM_ANDROID
	// Try to get Android ID via JNI
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (Env)
	{
		jclass Class = FAndroidApplication::FindJavaClass("android/provider/Settings$Secure");
		if (Class)
		{
			jmethodID Method = Env->GetStaticMethodID(Class, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
			if (Method)
			{
				jobject Activity = FAndroidApplication::GetGameActivityThis();
				jmethodID GetContentResolver = Env->GetMethodID(FAndroidApplication::FindJavaClass("android/app/Activity"), "getContentResolver", "()Landroid/content/ContentResolver;");
				
				if (GetContentResolver && Activity)
				{
					jobject ContentResolver = Env->CallObjectMethod(Activity, GetContentResolver);
					jstring AndroidIdKey = Env->NewStringUTF("android_id");
					jstring AndroidId = (jstring)Env->CallStaticObjectMethod(Class, Method, ContentResolver, AndroidIdKey);
					
					if (AndroidId)
					{
						const char* AndroidIdStr = Env->GetStringUTFChars(AndroidId, nullptr);
						ResultId = FString(UTF8_TO_TCHAR(AndroidIdStr));
						Env->ReleaseStringUTFChars(AndroidId, AndroidIdStr);
						ResultId = TEXT("AND-") + ResultId;
					}
					
					Env->DeleteLocalRef(AndroidIdKey);
					if (ContentResolver) Env->DeleteLocalRef(ContentResolver);
				}
			}
			Env->DeleteLocalRef(Class);
		}
	}
	
	if (ResultId.IsEmpty())
	{
		ResultId = GenerateFallbackUUID();
	}
#else
	ResultId = GenerateFallbackUUID();
#endif

	return ResultId;
}

FString UDeviceIdManager::GenerateFallbackUUID()
{
	// Generate a UUID-like identifier
	FGuid Guid = FGuid::NewGuid();
	return FString::Printf(TEXT("UUID-%s"), *Guid.ToString(EGuidFormats::DigitsWithHyphens));
}

