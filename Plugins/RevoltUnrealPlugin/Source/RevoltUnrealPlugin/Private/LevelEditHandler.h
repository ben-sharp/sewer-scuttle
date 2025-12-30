// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;
class UClass;

/**
 * Handles level editing operations including actor spawning and modification
 */
class FLevelEditHandler
{
public:

	/**
	 * Spawn an actor in a level
	 * @param World World/level to spawn actor in
	 * @param ActorClass Class of actor to spawn
	 * @param Location Spawn location
	 * @param Rotation Spawn rotation
	 * @param Scale Spawn scale
	 * @return Spawned actor or nullptr if failed
	 */
	static AActor* SpawnActorInLevel(UWorld* World, UClass* ActorClass, const FVector& Location, const FRotator& Rotation, const FVector& Scale);

	/**
	 * Configure properties on a spawned actor
	 * @param Actor Actor to configure
	 * @param Properties Map of property names to values
	 * @return True if successful
	 */
	static bool ConfigureActorProperties(AActor* Actor, const TMap<FString, FString>& Properties);

	/**
	 * Add tags to an actor
	 * @param Actor Actor to add tags to
	 * @param Tags Array of tag names
	 */
	static void AddActorTags(AActor* Actor, const TArray<FString>& Tags);

	/**
	 * Find an actor in a level by name
	 * @param World World to search
	 * @param ActorName Name of actor to find
	 * @return Found actor or nullptr
	 */
	static AActor* FindActorByName(UWorld* World, const FString& ActorName);

	/**
	 * Delete an actor from a level
	 * @param World World containing the actor
	 * @param Actor Actor to delete
	 * @return True if deleted successfully
	 */
	static bool DeleteActor(UWorld* World, AActor* Actor);

	/**
	 * Save level changes
	 * @param World World/level to save
	 * @return True if saved successfully
	 */
	static bool SaveLevel(UWorld* World);

	/**
	 * Load a level for editing (without opening it in editor)
	 * @param LevelName Name of level to load
	 * @return Loaded world or nullptr
	 */
	static UWorld* LoadLevelForEditing(const FString& LevelName);

private:

	/**
	 * Set property value on an actor
	 * @param Actor Actor to modify
	 * @param Property Property to set
	 * @param Value Value as string
	 * @return True if successful
	 */
	static bool SetActorPropertyValue(AActor* Actor, FProperty* Property, const FString& Value);
};

