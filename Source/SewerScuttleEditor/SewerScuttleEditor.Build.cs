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
			"SewerScuttle", // Our main game module
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"ToolMenus",
			"EditorStyle",
			"EditorWidgets",
			"Slate",
			"SlateCore",
			"LevelEditor",
			"PropertyEditor",
			"WorkspaceMenuStructure",
			"ContentBrowser",
			"AssetTools",
			"EditorSubsystem",
		});
	}
}

