// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ActionRPGTarget : TargetRules
{
	public ActionRPGTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "ActionRPG", "ActionRPGModio", "OSS_Provider" } );

		// For debug sym generation
		MacPlatform.bUseDSYMFiles = true;
        IOSPlatform.bGeneratedSYM = true;
    }
}
