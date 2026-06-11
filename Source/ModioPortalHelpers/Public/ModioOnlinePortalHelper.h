/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#if MODIO_PLATFORM_HELPERS
	#include COMPILED_PLATFORM_HEADER(ModioOnlinePortalHelper.h)
#else

#define CLIENT_ID_KEY "client_id"
#define SCOPE_KEY "scope"
#define LOCAL_USER_KEY "local_user"
#define DISPLAY_NAME_KEY "DisplayName"

#include "CoreTypes.h"

#include "OnlineSubsystem.h"
#include "GenericModioOnlinePortalImplementation.h"

DECLARE_DELEGATE_OneParam(FStringDelegate, FString);
DECLARE_DELEGATE_TwoParams(FEntitlementsRefreshedDelegate, bool, FString);

/** Modio Online Portal Helpers
 * This header contains a selection of helper functions intended for use with the
 * Modio Unreal Engine Plugin. They provide a single entry-point for interacting
 * with Online portal APIs for authentication, filepath handling, and monetization.
 **/
class FGenericModioOnlinePortalHelper
{
public:

	/**
	* For performing basic platform initialization, such as preparing the filesystem or
	* gathering user tokens
	**/
	static void InitializePlatform() {}

	/**
	* For preparing a specific filepath for use on the platform, e.g mounting, sanitizing
	* @param UgcPath The path the prepare for use
	* @returns Boolean intended to indicate whether the path was successfully prepared
	**/
	static bool PrepareFilesystemToUsePath(const FString& UgcPath)
	{
		return true;
	}

	/**
	* For getting the auth token for the current (or given via params) user, returning via Callback
	* @param Params platform-specific parameters for use in getting the auth token
	* @param Callback a callback delegate returning the auth token as an FString
	**/
	static void GetPlatformAuthToken(const TMap<FString, FString> Params, const FStringDelegate& Callback)
	{
		Callback.ExecuteIfBound(
			"FGenericModioOnlinePortalHelper::GetPlatformAuthToken called. You're probably in PiE.");
	}

	/**
	* Returns the client id for the current platform
	**/
	static FString GetClientId()
	{
		return "FGenericModioOnlinePortalHelper::GetClientId, You're probably in PiE.";
	}

	/**
	* Returns the current scope for the current platform
	**/
	static FString GetScope()
	{
		return "FGenericModioOnlinePortalHelper::GetScope, You're probably in PiE.";
	}

	/**
	* Returns platform-specific extendedinitialization params intended to be passed to the Modio Subsystem during init
	* @return A TMap of FString key-value pairs containing the paramaters 
	**/
	static TMap<FString, FString> GetExtendedInitializationParams()
	{
		return {{"FGenericModioOnlinePortalHelper::GetExtendedInitializationParams", "You're probably in PiE."}};
	}

	/**
	* For refreshing entitlements (premium currency/tokens) on the current platform, returning success/message via callback
	* @param Params platform-specific parameters for use when refreshing entitlements
	* @param Handler a delegate callback returning a boolean (success) and a string (message for error information)
	**/
	static void RefreshEntitlements(const TMap<FString, FString> Params, const FEntitlementsRefreshedDelegate& Handler)
	{
		Handler.ExecuteIfBound(false,
							   "FGenericModioOnlinePortalHelper::RefreshEntitlements called. You're probably in PiE.");
	}

	/**
	* Helper for sanitizing a given filepath for the current platform, for instances of case [in]sensitivity, slash direction, etc.
	* @param FilePath The filepath to sanitize
	* @return FString containing the sanitized filepath
	**/
	static FString SanitizeFilePath(FString& FilePath)
	{
		return FilePath;
	}

	/**
	* Returns platform-specific authentication params intended to be passed to Modio user authentication
	* @return A TMap of FString key-value pairs containing the paramaters
	**/
	static TMap<FString, FString> GetExtendedAuthParams()
	{
		return {{"FGenericModioOnlinePortalHelper::GetExtendedAuthParams", "You're probably in PiE."}};
	}
};
typedef FGenericModioOnlinePortalHelper FModioOnlinePortalHelper;

#endif