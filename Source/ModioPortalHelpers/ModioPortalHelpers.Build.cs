using System;
using UnrealBuildTool;

public class ModioPortalHelpers : ModuleRules
{
    protected virtual bool bHasModioPortalHelpers => false;
    public ModioPortalHelpers(ReadOnlyTargetRules Target) : base(Target)
    {
       PublicDefinitions.Add("MODIO_PLATFORM_HELPERS=" + (bHasModioPortalHelpers ? "1" : "0"));

       PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Modio",
                "ModioUICore",
                "CoreOnline"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "OnlineSubsystem",
                "OnlineSubsystemUtils"
            });
    }
}