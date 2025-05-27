/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "Types/ModioInitializeOptions.h"

#include "ActionRPGModioSubsystem.generated.h"

/** Handles initialization of the core mod.io plugin, as well as ModioUGC provider and discovery/loading of UGC
 * Also handles launch arguments for changing parameters at startup.
 */
UCLASS()
class ACTIONRPGMODIO_API UActionRPGModioSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UActionRPGModioSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UFUNCTION()
	void OnUGCProviderInitialized(bool bSuccess);

	/**
	 * Uses the SteamAPI to add the given item to the currently authenticated user's steam inventory, in the given
	 * quantity
	 * @param Result The result returned from the Steam API, -1 indicates an error.
	 * @param ItemId The ID of the Item to add, defaults to 101 which is the 200 token pack in the Monetization Test
	 * Game
	 * @param Quantity How many of the item to add to the user's inventory.
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|Utilities|SteamApi")
	static void AddItemToSteamInventory(int32& Result, int32 ItemId = 101, int32 Quantity = 1);

	/**
	 * Get the game environment
	 * @param OutEnvironment The environment to get
	 * @return true if the environment was overridden, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|mod.io|Utilities")
	bool GetOverrideGameEnvironment(EModioEnvironment& OutEnvironment) const
	{
		if (OverrideGameEnvironment.IsSet())
		{
			OutEnvironment = OverrideGameEnvironment.GetValue();
			return true;
		}
		return false;
	}

	/**
	 * Get the override portal value
	 * @param OutPortal The output parameter to receive the portal value
	 * @return true if the portal was overridden, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|mod.io|Utilities")
	bool GetOverridePortal(EModioPortal& OutPortal) const
	{
		if (OverridePortal.IsSet())
		{
			OutPortal = OverridePortal.GetValue();
			return true;
		}
		return false;
	}

private:

	/**
	 * Invoked before mods are uninstalled, providing an opportunity to veto the uninstall.
	 * Ensures the Mod is unloaded before approval.
	 * @param FModioModID The ID of the mod we're going to uninstall.
	 * @return true if the uninstall is approved, false otherwise
	 */
	UFUNCTION()
	bool OnModPreUninstallCallback(FModioModID ModioModID);
	
	/**
	 * Get the override value for mod storage quota in megabytes
	 * @param OutModStorageQuotaMB The output parameter to receive the quota value
	 * @return true if the mod storage quota was overridden, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|mod.io|Utilities")
	bool GetOverrideModStorageQuotaMB(int32& OutModStorageQuotaMB) const
	{
		if (OverrideModStorageQuotaMB.IsSet())
		{
			OutModStorageQuotaMB = OverrideModStorageQuotaMB.GetValue();
			return true;
		}
		return false;
	}

	/**
	 * Get the override value for image cache storage quota in megabytes
	 * @param OutCacheStorageQuotaMB The output parameter to receive the quota value
	 * @return true if the cache storage quota was overridden, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|mod.io|Utilities")
	bool GetOverrideCacheStorageQuotaMB(int32& OutCacheStorageQuotaMB) const
	{
		if (OverrideCacheStorageQuotaMB.IsSet())
		{
			OutCacheStorageQuotaMB = OverrideCacheStorageQuotaMB.GetValue();
			return true;
		}
		return false;
	}

	/**
	 * Create a duplicate of initialization options with our override values
	 * @param Options The initialization options to augment
	 * @return A duplicate of the initialize options with override values
	 */
	UFUNCTION(BlueprintCallable, Category = "ActionRPG|mod.io|Utilities")
	FModioInitializeOptions OverrideInitializationOptions(const FModioInitializeOptions& Options) const;

	FModioInitializeOptions GetModioInitializeOptions() const;

private:
	/**
	 * Override GameId parsed from commandline parameters
	 */
	int32 OverrideGameId = -1;

	/**
	 * Override API Key parsed from commandline parameters
	 */
	FString OverrideApiKey;

	/**
	 * Override Metrics Secret Key parsed from commandline parameters
	 */
	FString OverrideMetricsSecretKey;

	/**
	 * Override game environment. Unreal Property System doesn't support TOptional, so this is accessed via
	 * GetOverrideGameEnvironment
	 */
	TOptional<EModioEnvironment> OverrideGameEnvironment;

	/**
	 * Override portal in use. Unreal Property System doesn't support TOptional, so this is accessed via
	 * GetOverridePortal
	 */
	TOptional<EModioPortal> OverridePortal;

	/**
	 * Override URL parsed from commandline parameters
	 */
	FString OverrideUrl;

	/**
	 * Flag for whether we have overridden launch options, as a convenience check
	 */
	bool HasOverriddenLaunchOptions;

	/**
	 * Override Session ID parsed from commandline parameters
	 */
	FString OverrideSessionID;

	/**
	 * Flag for overriding the state of the mod enable/disable feature
	 */
	bool bOverrideModEnableDisableFeature;

	/**
	 * Flag for overriding the state of the monetization feature
	 */
	bool bOverrideMonetizationFeature;

	/**
	 * Flag for overriding the state of the mod downvote feature
	 */
	bool bOverrideDownvoteFeature;

	/**
	 * Override for mod storage quota in megabytes (minimum 25 MB)
	 */
	TOptional<int32> OverrideModStorageQuotaMB;

	/**
	 * Override for cache storage quota in megabytes (minimum 25 MB)
	 */
	TOptional<int32> OverrideCacheStorageQuotaMB;
};