// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ActionRPGServerTarget : TargetRules
{
	public ActionRPGServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		ExtraModuleNames.AddRange( new string[] { "ActionRPG", "ActionRPGModio", "OSS_Provider" } );

		//ProjectDefinitions.Add("MODIO_DEVELOPMENT_MODE");
	}
}
