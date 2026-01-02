// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SewerScuttleEditor : ModuleRules
{
	public SewerScuttleEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"SewerScuttle" 
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate", 
			"SlateCore", 
			"UnrealEd", 
			"ToolMenus", 
			"LevelEditor" 
		});
	}
}

