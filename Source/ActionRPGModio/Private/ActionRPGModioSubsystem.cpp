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
	FString APIKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("apikey="), APIKey))
	{
		OverrideApiKey = APIKey;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using APIKey: %s"), *APIKey);
		HasOverriddenLaunchOptions = true;
	}

	int32 GameID;
	if (FParse::Value(FCommandLine::Get(), TEXT("gameid="), GameID))
	{
		OverrideGameId = GameID;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using GameID: %i"), GameID);
		HasOverriddenLaunchOptions = true;
	}

	FString MetricsSecretKey;
	if (FParse::Value(FCommandLine::Get(), TEXT("metricssecretkey="), MetricsSecretKey))
	{
		OverrideMetricsSecretKey = MetricsSecretKey;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using MetricsSecretKey: %s"), *MetricsSecretKey);
		HasOverriddenLaunchOptions = true;
	}

	EModioEnvironment GameEnvironment;
	FString GameEnvironmentStr;
	if (FParse::Value(FCommandLine::Get(), TEXT("env="), GameEnvironmentStr))
	{
		if (UModioHelpers::StringToModioGameEnvironment(GameEnvironmentStr, GameEnvironment))
		{
			OverrideGameEnvironment = GameEnvironment;
			UE_LOG(LogActionRPGModio, Display, TEXT("Using GameEnvironment: %s"), *GameEnvironmentStr);
			HasOverriddenLaunchOptions = true;
		}
	}

	EModioPortal Portal;
	FString PortalStr;
	if (FParse::Value(FCommandLine::Get(), TEXT("portal="), PortalStr) &&
		UModioHelpers::StringToModioPortal(PortalStr, Portal))
	{
		OverridePortal = Portal;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using Portal: %s"), *PortalStr);
		HasOverriddenLaunchOptions = true;
	}

	FString Url;
	if (FParse::Value(FCommandLine::Get(), TEXT("url="), Url))
	{
		OverrideUrl = Url;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using URL: %s"), *Url);
		HasOverriddenLaunchOptions = true;
	}

	FString SessionID;
	if (FParse::Value(FCommandLine::Get(), TEXT("sessionid="), SessionID))
	{
		OverrideSessionID = SessionID;
		UE_LOG(LogActionRPGModio, Display, TEXT("Using SessionID: %s"), *SessionID);
		HasOverriddenLaunchOptions = true;
	}

	FString EnableDisableState;
	if (FParse::Value(FCommandLine::Get(), TEXT("enabledisable="), EnableDisableState))
	{
		bOverrideModEnableDisableFeature = EnableDisableState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Enable/Disable Feature enabled?: %hs"),
			   bOverrideModEnableDisableFeature ? "true" : "false");
		GetMutableDefault<UModioSettings>()->bEnableModEnableDisableFeature = bOverrideModEnableDisableFeature;
	}

	FString MonetizationState;
	if (FParse::Value(FCommandLine::Get(), TEXT("monetization="), MonetizationState))
	{
		bOverrideMonetizationFeature = MonetizationState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Monetization Feature enabled?: %hs"),
			   bOverrideMonetizationFeature ? "true" : "false");
		GetMutableDefault<UModioSettings>()->bEnableMonetizationFeature = bOverrideMonetizationFeature;
	}

	FString DownvoteState;
	if (FParse::Value(FCommandLine::Get(), TEXT("downvote="), DownvoteState))
	{
		bOverrideDownvoteFeature = DownvoteState.ToBool();
		UE_LOG(LogActionRPGModio, Display, TEXT("Downvote Feature enabled?: %hs"),
			   bOverrideDownvoteFeature ? "true" : "false");
		GetMutableDefault<UModioSettings>()->bEnableModDownvoteFeature = bOverrideDownvoteFeature;
		HasOverriddenLaunchOptions = true;
	}

	FString ModStorageQuotaMB;
	if (FParse::Value(FCommandLine::Get(), TEXT("modstoragequota="), ModStorageQuotaMB))
	{
		OverrideModStorageQuotaMB = FCString::Atoi(*ModStorageQuotaMB);
		UE_LOG(LogActionRPGModio, Display, TEXT("Mod Storage Quota: %i"), OverrideModStorageQuotaMB.GetValue());
		HasOverriddenLaunchOptions = true;
	}

	FString CacheStorageQuotaMB;
	if (FParse::Value(FCommandLine::Get(), TEXT("cachestoragequota="), CacheStorageQuotaMB))
	{
		OverrideCacheStorageQuotaMB = FCString::Atoi(*CacheStorageQuotaMB);
		UE_LOG(LogActionRPGModio, Display, TEXT("Cache Storage Quota: %i"), OverrideCacheStorageQuotaMB.GetValue());
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
	uint32 InnerCount = uint32(Quantity);
	UE_LOG(LogActionRPGModio, Warning, TEXT("Adding item to inventory"));
	SteamInventory()->GenerateItems(&InnerResult, &InnerItem, &InnerCount, 1);
#else
	UE_LOG(LogActionRPGModio, Warning,
		   TEXT("AddItemToSteamInventory is only supported on Steam-compatible platforms."));
#endif
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

	FModioInitializeOptions DuplicateOptions {Options};

	if (OverrideGameId > 0)
	{
		DuplicateOptions.GameId = FModioGameID(OverrideGameId);
	}
	if (!OverrideApiKey.IsEmpty())
	{
		DuplicateOptions.ApiKey = FModioApiKey(OverrideApiKey);
	}
	if (!OverrideMetricsSecretKey.IsEmpty())
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("MetricsSecretKey", OverrideMetricsSecretKey);
	}
	EModioEnvironment GameEnvironment;
	if (GetOverrideGameEnvironment(GameEnvironment))
	{
		DuplicateOptions.GameEnvironment = GameEnvironment;
	}
	if (!OverrideUrl.IsEmpty())
	{
		// "EnvironmentOverrideUrl" was taken from Modio::Detail::HttpService::ApplyGlobalConfigOverrides
		DuplicateOptions.ExtendedInitializationParameters.Add("EnvironmentOverrideUrl", OverrideUrl);
	}
	if (!OverrideSessionID.IsEmpty())
	{
		DuplicateOptions.LocalSessionIdentifier = OverrideSessionID;
	}

	int32 ModStorageQuotaMB;
	if (GetOverrideModStorageQuotaMB(ModStorageQuotaMB))
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("ModStorageQuotaMB", FString::FromInt(ModStorageQuotaMB));
	}

	int32 CacheStorageQuotaMB;
	if (GetOverrideCacheStorageQuotaMB(CacheStorageQuotaMB))
	{
		DuplicateOptions.ExtendedInitializationParameters.Add("CacheStorageQuotaMB",
															  FString::FromInt(CacheStorageQuotaMB));
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