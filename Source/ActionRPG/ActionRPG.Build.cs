// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ActionRPG : ModuleRules
{
	public ActionRPG(ReadOnlyTargetRules Target)
		: base(Target)
	{
		PrivatePCHHeaderFile = "Public/ActionRPG.h";

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"CommonUI",
				"ModioUGC"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"ActionRPGLoadingScreen",
				"ActionRPGModio",
				"ModioPortalHelpers",
				"Slate",
				"SlateCore",
				"InputCore",
				"MoviePlayer",
				"GameplayAbilities",
				"GameplayTags",
				"GameplayTasks",
				"AIModule",
				"UMG"
			}
		);

		PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemUtils" });

		if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
		{
			PublicDefinitions.Add("WITH_STEAM");

			PublicDependencyModuleNames.AddRange(new string[]
			{
				"OnlineSubsystemSteam",
				"SteamShared"
			});
			PrivateDependencyModuleNames.AddRange(new string[]
			{
				"OnlineSubsystemSteam",
				"SteamShared"
			});

			AddEngineThirdPartyPrivateStaticDependencies(Target, "Steamworks");
		}

		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("Launch");
		}

		if ((Target.Platform == UnrealTargetPlatform.Win64)
			|| (Target.Platform == UnrealTargetPlatform.Linux)
			|| (Target.Platform == UnrealTargetPlatform.Mac)
			|| (Target.Platform == UnrealTargetPlatform.Android)
			|| (Target.Platform == UnrealTargetPlatform.IOS))
		{
			PublicDependencyModuleNames.Add("Sentry");
		}

		//Set up mutator subclass for module
        PublicDefinitions.Add("MUTATOR_CLASS=URPGMutator");
        PublicDefinitions.Add("MUTATOR_SUBSYSTEM_CLASS=URPGMutatorSubsystem");
    }
}
