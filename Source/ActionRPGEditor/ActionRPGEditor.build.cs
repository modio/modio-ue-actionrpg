using UnrealBuildTool;
 
public class ActionRPGEditor : ModuleRules
{
	public ActionRPGEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "UnrealEd", "Blutility","DesktopPlatform","SlateCore","Slate","Json", "UMG"});
 
		PublicIncludePaths.AddRange(new string[] {"ActionRPGEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"ActionRPGEditor/Private"});
	}
}