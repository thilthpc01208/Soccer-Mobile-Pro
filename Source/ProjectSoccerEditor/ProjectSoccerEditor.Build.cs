using UnrealBuildTool;

public class ProjectSoccerEditor : ModuleRules
{
	public ProjectSoccerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"Engine",
			"CoreUObject",
			"ProjectSoccer",
			"RHI",
			"RenderCore",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"DataTableEditor",
			"Json",
			"JsonUtilities",
			"AssetRegistry"
		});
	}
}
