using UnrealBuildTool;

public class ActionRPGTests : ModuleRules
{
	public ActionRPGTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"ActionRPG",
			"ActionRPGModio",
			"ModioPortalHelpers"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"ModioUGC",
			"UnrealEd",
			"AssetRegistry"
		});
	}
}
