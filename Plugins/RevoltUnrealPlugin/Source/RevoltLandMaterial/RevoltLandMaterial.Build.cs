using UnrealBuildTool;

public class RevoltLandMaterial : ModuleRules
{
	public RevoltLandMaterial(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"MaterialEditor"
				}
			);
		}

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Landscape",
				"AssetRegistry"
			}
		);
	}
}

