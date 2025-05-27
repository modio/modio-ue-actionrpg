/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreTypes.h"
#include "ModioEntitlementSubsystem.h"

#include "UObject/UObjectGlobals.h"

#include "ModioPlatformHelpers.generated.h"

class IOnlineSubsystem;
DECLARE_DELEGATE_OneParam(FOnGetAuthTokenCompletedDelegate, FString);

/**
 * A generic platform helper that should be used as the default if there is not a
 * platform specific customization. */
UCLASS()
class UGenericModioPlatformHelper : public UObject
{
	GENERATED_BODY()

public:
	static void GetPlatformAuthToken(const TMap<FString, FString> Params,
									 const FOnGetAuthTokenCompletedDelegate& Callback)
	{
		Callback.ExecuteIfBound(
			"FGenericModioPlatformHelper::GetPlatformAuthToken called. You shouldn't ever see this.");
	}

	static FString GetClientId()
	{
		return "Generic Client ID, you should not be seeing this.";
	}

	static FString GetScope()
	{
		return "Generic Scope, you should not be seeing this.";
	}

	static TMap<FString, FString> GetExtendedInitializationParams()
	{
		return {};
	}

	static void RefreshEntitlements(IOnlineSubsystem* OnlineSubsystem, UModioSubsystem* ModioSubsystem,
									UModioUISubsystem* UISubsystem, const FOnUserEntitlementsRefreshedDelegate& Handler)
	{
		Handler.ExecuteIfBound(false,
							   "GenericModioPlatformHelper::RefreshEntitlements called. You shouldn't ever see this.");
	}

	static FString SanitizeFilePath(FString& FilePath)
	{
		return FilePath;
	}
};

#if MODIO_USE_PLATFORM_HELPERS

	#include COMPILED_PLATFORM_HEADER(ModioPlatformHelper.h)

#else

typedef UGenericModioPlatformHelper UModioPlatformHelper;

#endif