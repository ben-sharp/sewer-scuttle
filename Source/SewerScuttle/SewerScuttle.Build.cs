// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SewerScuttle : ModuleRules
{
	public SewerScuttle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"Slate",
			"SlateCore",
			"HTTP",
			"Json",
			"JsonUtilities",
			"GameplayAbilities",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Add include paths for subdirectories so cross-directory includes work
		PublicIncludePaths.Add(ModuleDirectory);
		PublicIncludePaths.Add(ModuleDirectory + "/EndlessRunner");
		PublicIncludePaths.Add(ModuleDirectory + "/UI");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
