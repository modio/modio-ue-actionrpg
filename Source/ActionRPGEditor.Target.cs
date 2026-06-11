// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;

public class ActionRPGEditorTarget : TargetRules
{
	public ActionRPGEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        if (Directory.Exists("./ModioDebugUI"))
        {
			ExtraModuleNames.AddRange( new string[] { "ModioDebugUI" } );
		}
		ExtraModuleNames.AddRange( new string[] { "ActionRPGEditor", "ActionRPG", "ActionRPGModio", "OSS_Provider", "ModioPortalHelpers" } );
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;

		//ProjectDefinitions.Add("MODIO_DEVELOPMENT_MODE");
		if (bBuildEditor)
		{
			ExtraModuleNames.Add("ActionRPGTests");
		}
	}
}
