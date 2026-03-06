// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using UnrealBuildTool;

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
        string ExternalVersionHash = Environment.GetEnvironmentVariable("BUILD_ID");

        if (ExternalVersionHash != null)
        {
			ProjectDefinitions.Add(String.Format("BUILD_ID={0}", ExternalVersionHash));
        }
    }
}
