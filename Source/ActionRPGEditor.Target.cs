// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ActionRPGEditorTarget : TargetRules
{
	public ActionRPGEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.AddRange( new string[] { "ActionRPGEditor", "ActionRPG", "ActionRPGModio", "OSS_Provider" } );
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		//ProjectDefinitions.Add("MODIO_DEVELOPMENT_MODE");
    	}
	
}
