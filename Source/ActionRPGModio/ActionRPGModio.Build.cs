using System;
using UnrealBuildTool;

public class ActionRPGModio : ModuleRules
{
    protected virtual bool bPlatformHasModioHelpers => false;

    public ActionRPGModio(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Modio",
                "ModioUGC"
            });

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "ModioUICore",
                "ModioUI",
                "UMG"
            });

        PublicDefinitions.Add("MODIO_USE_PLATFORM_HELPERS=" + (bPlatformHasModioHelpers ? "1" : "0"));

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) ||
            (Target.Platform == UnrealTargetPlatform.Mac))
        {
            PublicDefinitions.Add("WITH_STEAM");

            AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
        }

        var UseMetaPlugin = Environment.GetEnvironmentVariable("USE_META_PLUGIN");
        if (UseMetaPlugin != null && UseMetaPlugin.Equals("true", StringComparison.CurrentCultureIgnoreCase))
        {
            PublicDefinitions.Add("USE_META_PLUGIN");

            PublicDependencyModuleNames.Add("OVRPlatform");
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");
        }
    }
}
