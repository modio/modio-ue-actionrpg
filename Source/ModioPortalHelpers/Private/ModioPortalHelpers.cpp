/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ModioPortalHelpers.h"

#include "ModioOnlinePortalHelper.h"

static FAutoConsoleCommand CmdTestSso(
	TEXT("Modio.Online.TestSSO"), TEXT("Test SSO with custom arguments for scope and client. Args: client_id"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (Args.Num() == 1)
		{
			/*TMap<FString, FString> Params;
			Params.Add("client_id", Args[0]);
			FModioOnlinePortalHelper::GetPlatformAuthToken(
				Params, FStringDelegate::CreateLambda([](const FString& Result)
					{
						UE_LOG(LogTemp, Warning, TEXT("Response to SSO Test: %s"), *Result);
					}));*/
		}
	}));

IMPLEMENT_MODULE(FModioPortalHelpersModule, ModioPortalHelpers)

/** Logging definitions */
DEFINE_LOG_CATEGORY(LogModioPortalHelpers);
