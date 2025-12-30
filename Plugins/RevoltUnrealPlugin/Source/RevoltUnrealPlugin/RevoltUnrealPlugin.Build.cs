// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RevoltUnrealPlugin : ModuleRules
{
	public RevoltUnrealPlugin(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Json",
				"JsonUtilities",
				"HTTP",
				"HTTPServer",
				"UnrealEd",
				"AssetRegistry",
				"Slate",
				"SlateCore",
				"ToolMenus",
				"DeveloperSettings",
				"KismetCompiler",
				"AssetTools",
				"EditorSubsystem",
				"Projects",
				"RevoltLandGen",
			}
		);
		
		// BlueprintGraph is optional - only add if available
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.Add("BlueprintGraph");
		}
	}
}

