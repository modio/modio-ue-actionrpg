/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGModioSubsystem.h"

#include "ActionRPGModio.h"
#include "ActionRPG_UGCProvider.h"
#include "Internationalization/Culture.h"
#include "Libraries/ModioPlatformHelpersLibrary.h"
#include "Libraries/ModioSDKLibrary.h"
#include "ModioHelpers.h"
#include "ModioPlatformHelpers.h"
#include "ModioSettings.h"
#include "ModioSubsystem.h"
#include "ModioUISubsystem.h"
#include "OnlineSubsystem.h"
#include "UGC/ModioUGCProvider.h"
#include "UGC/UGCSubsystem.h"

#ifdef WITH_STEAM
THIRD_PARTY_INCLUDES_START
#include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END
#endif

UActionRPGModioSubsystem::UActionRPGModioSubsystem()
{
	UModioSettings& ModioSettings = *GetMutableDefault<UModioSettings>();

	FString APIKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("apikey="), APIKey))
	{
		ModioSettings.OverrideApiKey = APIKey;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using APIKey: %s"), *APIKey);
		HasOverriddenLaunchOptions = true;
	}

	int32 GameID;
	if (FParse::Value(FCommandLine::Get(), TEXT("gameid="), GameID))
	{
		ModioSettings.OverrideGameId = GameID;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using GameID: %i"), GameID);
		HasOverriddenLaunchOptions = true;
	}

	FString MetricsSecretKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("metricssecretkey="), MetricsSecretKey))
	{
		ModioSettings.OverrideMetricsSecretKey = MetricsSecretKey;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using MetricsSecretKey: %s"), *MetricsSecretKey);
		HasOverriddenLaunchOptions = true;
	}

	EModioEnvironment GameEnvironment;
	FString GameEnvironmentStr;
	if (FParse::Value(FCommandLine::Get(), TEXT("env="), GameEnvironmentStr))
	{
		if (UModioHelpers::StringToModioGameEnvironment(GameEnvironmentStr, GameEnvironment))
		{
			ModioSettings.OverrideGameEnvironment = GameEnvironment;
			UE_LOG(LogActionRPGModio, Display, TEXT("Using GameEnvironment: %s"), *GameEnvironmentStr);
			HasOverriddenLaunchOptions = true;
		}
	}

	EModioPortal Portal;
	FString PortalStr;
	if (FParse::Value(FCommandLine::Get(), TEXT("portal="), PortalStr) &&
	    UModioHelpers::StringToModioPortal(PortalStr, Portal))
	{
		ModioSettings.OverridePortal = Portal;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using Portal: %s"), *PortalStr);
		HasOverriddenLaunchOptions = true;
	}

	FString Url;
	if (FParse::Value(FCommandLine::Get(), TEXT("url="), Url))
	{
		ModioSettings.OverrideUrl = Url;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using URL: %s"), *Url);
		HasOverriddenLaunchOptions = true;
	}

	FString SessionID;
	if (FParse::Value(FCommandLine::Get(), TEXT("sessionid="), SessionID))
	{
		ModioSettings.OverrideSessionID = SessionID;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using SessionID: %s"), *SessionID);
		HasOverriddenLaunchOptions = true;
	}

	FString EnableDisableState;
	if (FParse::Value(FCommandLine::Get(), TEXT("enabledisable="), EnableDisableState))
	{
		ModioSettings.bEnableModEnableDisableFeature = EnableDisableState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Enable/Disable Feature enabled?: %hs"),
		       ModioSettings.bEnableModEnableDisableFeature ? "true" : "false");
	}

	FString MonetizationState;
	if (FParse::Value(FCommandLine::Get(), TEXT("monetization="), MonetizationState))
	{
		ModioSettings.bEnableMonetizationFeature = MonetizationState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Monetization Feature enabled?: %hs"),
		       ModioSettings.bEnableMonetizationFeature ? "true" : "false");
	}

	FString DownvoteState;
	if (FParse::Value(FCommandLine::Get(), TEXT("downvote="), DownvoteState))
	{
		ModioSettings.bEnableModDownvoteFeature = DownvoteState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Downvote Feature enabled?: %hs"),
		       ModioSettings.bEnableModDownvoteFeature ? "true" : "false");
		HasOverriddenLaunchOptions = true;
	}

	FString ModStorageQuotaMB;
	if (FParse::Value(FCommandLine::Get(), TEXT("modstoragequota="), ModStorageQuotaMB))
	{
		ModioSettings.OverrideModStorageQuotaMB = FCString::Atoi(*ModStorageQuotaMB);
		UE_LOG(LogActionRPGModio, Display, TEXT("Mod Storage Quota: %i"), *ModioSettings.OverrideModStorageQuotaMB);
		HasOverriddenLaunchOptions = true;
	}

	FString CacheStorageQuotaMB;
	if (FParse::Value(FCommandLine::Get(), TEXT("cachestoragequota="), CacheStorageQuotaMB))
	{
		ModioSettings.OverrideCacheStorageQuotaMB = FCString::Atoi(*CacheStorageQuotaMB);
		UE_LOG(LogActionRPGModio, Display, TEXT("Cache Storage Quota: %i"), *ModioSettings.OverrideCacheStorageQuotaMB);
		HasOverriddenLaunchOptions = true;
	}
}

void UActionRPGModioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency(UModioSubsystem::StaticClass());
	Collection.InitializeDependency(UUGCSubsystem::StaticClass());
	Collection.InitializeDependency(UModioUISubsystem::StaticClass());

	// Initialize the SDK
	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		UUGCSubsystem* UGCSubsystem = GEngine->GetEngineSubsystem<UUGCSubsystem>();
		UGCSubsystem->SetFilePathSanitizationFn(UModioPlatformHelper::SanitizeFilePath);
		// Set the language code for the plugin to use
		const EModioLanguage CurrentLanguage = UModioSDKLibrary::GetLanguageCodeFromString(
			FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName());

		ModioSubsystem->SetLanguage(CurrentLanguage);

		ModioSubsystem->InitializeAsync(
			GetModioInitializeOptions(),
			FOnErrorOnlyDelegateFast::CreateLambda([this, UGCSubsystem](FModioErrorCode ec) {
				if (ec)
				{
					UE_LOG(LogActionRPGModio, Error, TEXT("Failed to initialize Mod.io SDK with error: %s"),
					       *ec.GetErrorMessage());
				}
				else
				{
					if (UGCSubsystem)
					{
						UGCSubsystem->SetUGCProvider(NewObject<UActionRPG_UGCProvider>(this));

						FOnUGCProviderInitializedDelegate Handler;
						Handler.BindDynamic(this, &UActionRPGModioSubsystem::OnUGCProviderInitialized);
						UGCSubsystem->InitializeUGCProvider(Handler);
					}
				}
			}));
	}

	// Register a delegate to unload UGC when uninstalling a mod (via UI)
	if (UModioUISubsystem* ModioUISubsystem = GEngine->GetEngineSubsystem<UModioUISubsystem>())
	{
		FOnPreUninstallDelegate Callback;
		Callback.BindDynamic(this, &UActionRPGModioSubsystem::OnModPreUninstallCallback);
		ModioUISubsystem->RegisterPreUninstallHandler(Callback);
	}
}

void UActionRPGModioSubsystem::OnUGCProviderInitialized(bool bSuccess)
{
	UE_LOG(LogActionRPGModio, Log, TEXT("UGC Provider initialized for RPG Modio Subsystem."));
}

void UActionRPGModioSubsystem::AddItemToSteamInventory(int32& Result, int32 ItemId, int32 Quantity)
{
	#ifdef WITH_STEAM
	SteamInventoryResult_t InnerResult = Result;
	SteamItemDef_t InnerItem = ItemId;
	uint32 InnerCount = static_cast<uint32>(Quantity);
	UE_LOG(LogActionRPGModio, Warning, TEXT("Adding item to inventory"));
	SteamInventory()->GenerateItems(&InnerResult, &InnerItem, &InnerCount, 1);
	#else
	UE_LOG(LogActionRPGModio, Warning,
		   TEXT("AddItemToSteamInventory is only supported on Steam-compatible platforms."));
	#endif
}

bool UActionRPGModioSubsystem::GetOverrideGameEnvironment(EModioEnvironment& OutEnvironment) const
{
	const UModioSettings& ModioSettings = *GetDefault<UModioSettings>();
	if (ModioSettings.OverrideGameEnvironment.IsSet())
	{
		OutEnvironment = ModioSettings.OverrideGameEnvironment.GetValue();
		return true;
	}
	return false;
}

bool UActionRPGModioSubsystem::GetOverridePortal(EModioPortal& OutPortal) const
{
	const UModioSettings& ModioSettings = *GetDefault<UModioSettings>();
	if (ModioSettings.OverridePortal.IsSet())
	{
		OutPortal = ModioSettings.OverridePortal.GetValue();
		return true;
	}
	return false;
}

bool UActionRPGModioSubsystem::OnModPreUninstallCallback(FModioModID ModioModID)
{
	if (UUGCSubsystem* UGCSubsystem = GEngine->GetEngineSubsystem<UUGCSubsystem>())
	{
		return UGCSubsystem->UnloadUGCByModID(FGenericModID(GetUnderlyingValue(ModioModID)));
	}
	return false;
}

FModioInitializeOptions UActionRPGModioSubsystem::OverrideInitializationOptions(
	const FModioInitializeOptions& Options) const
{
	if (!HasOverriddenLaunchOptions)
	{
		return Options;
	}

	const UModioSettings& ModioSettings = *GetDefault<UModioSettings>();
	FModioInitializeOptions DuplicateOptions{Options};

	if (ModioSettings.OverrideGameId.IsSet())
	{
		DuplicateOptions.GameId = FModioGameID(*ModioSettings.OverrideGameId);
	}
	if (ModioSettings.OverrideApiKey.IsSet())
	{
		DuplicateOptions.ApiKey = FModioApiKey(*ModioSettings.OverrideApiKey);
	}
	if (ModioSettings.OverrideMetricsSecretKey.IsSet())
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("MetricsSecretKey",
		                                                      *ModioSettings.OverrideMetricsSecretKey);
	}
	if (ModioSettings.OverrideGameEnvironment.IsSet())
	{
		DuplicateOptions.GameEnvironment = *ModioSettings.OverrideGameEnvironment;
	}
	if (ModioSettings.OverrideUrl.IsSet())
	{
		// "EnvironmentOverrideUrl" was taken from Modio::Detail::HttpService::ApplyGlobalConfigOverrides
		DuplicateOptions.ExtendedInitializationParameters.Add("EnvironmentOverrideUrl", *ModioSettings.OverrideUrl);
	}
	if (ModioSettings.OverrideSessionID.IsSet())
	{
		DuplicateOptions.LocalSessionIdentifier = *ModioSettings.OverrideSessionID;
	}

	if (ModioSettings.OverrideModStorageQuotaMB.IsSet())
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("ModStorageQuotaMB",
		                                                      FString::FromInt(
			                                                      *ModioSettings.OverrideModStorageQuotaMB));
	}

	if (ModioSettings.OverrideCacheStorageQuotaMB.IsSet())
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("CacheStorageQuotaMB",
		                                                      FString::FromInt(
			                                                      *ModioSettings.OverrideCacheStorageQuotaMB));
	}

	return DuplicateOptions;
}

FModioInitializeOptions UActionRPGModioSubsystem::GetModioInitializeOptions() const
{
	FModioInitializeOptions Options =
		UModioSDKLibrary::GetProjectInitializeOptionsForSessionId(FPlatformProcess::UserName());

	Options.PortalInUse = UModioPlatformHelpersLibrary::GetDefaultPortalForCurrentPlatform();
	Options.ExtendedInitializationParameters.Append(UModioPlatformHelper::GetExtendedInitializationParams());
	Options = OverrideInitializationOptions(Options);

	return Options;
}