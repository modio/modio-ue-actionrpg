/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ModioEntitlementSubsystem.h"

#include "ModioPlatformHelpers.h"
#include "ModioSubsystem.h"
#include "ModioUISubsystem.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "Libraries/ModioPlatformHelpersLibrary.h"
#include "OnlineSubsystem.h"

void UModioEntitlementSubsystem::NativeOnEntitlementRefreshEvent()
{
	IModioUIEntitlementRefreshEventReceiver::NativeOnEntitlementRefreshEvent();
	RefreshUserEntitlements();
}

void UModioEntitlementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	IModioUIEntitlementRefreshEventReceiver::Register<UModioEntitlementSubsystem>();
}

void UModioEntitlementSubsystem::RefreshUserEntitlements()
{
	RefreshUserEntitlementsWithHandler({});
}

void UModioEntitlementSubsystem::RefreshUserEntitlementsWithHandler(const FOnUserEntitlementsRefreshedDelegate& Handler)
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::GetByPlatform();
	UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
	UModioUISubsystem* UISubsystem = GEngine->GetEngineSubsystem<UModioUISubsystem>();

	if (OnlineSubsystem && ModioSubsystem && UISubsystem)
	{
		switch (UModioPlatformHelpersLibrary::GetDefaultPortalForCurrentPlatform())
		{
			case EModioPortal::Steam:
			{
				RefreshEntitlementsSteam(OnlineSubsystem, ModioSubsystem, UISubsystem, Handler);
				break;
			}
			case EModioPortal::XboxLive:
			case EModioPortal::PSN:
			{
				UModioPlatformHelper::RefreshEntitlements(OnlineSubsystem, ModioSubsystem, UISubsystem, Handler);
				break;
			}
			case EModioPortal::None:
			case EModioPortal::Apple:
			case EModioPortal::EpicGamesStore:
			case EModioPortal::GOG:
			case EModioPortal::Google:
			case EModioPortal::Itchio:
			case EModioPortal::Nintendo:
			default:
			{
				Handler.ExecuteIfBound(false, "Could not find Entitlement Refresh method for current portal.");
			}
		}
	}
}

void UModioEntitlementSubsystem::RefreshEntitlementsSteam(IOnlineSubsystem* OnlineSubsystem,
														  TWeakObjectPtr<UModioSubsystem> ModioSubsystem,
														  TWeakObjectPtr<UModioUISubsystem> ModioUISubsystem,
														  const FOnUserEntitlementsRefreshedDelegate& Handler)
{
	if (OnlineSubsystem->GetPurchaseInterface().IsValid() && OnlineSubsystem->GetIdentityInterface().IsValid())
	{
		IOnlineIdentity::FOnGetLinkedAccountAuthTokenCompleteDelegate LinkedAccountAuthCallback;
		LinkedAccountAuthCallback.BindLambda([ModioSubsystem, ModioUISubsystem,
											  Handler](int32 LocalUserNum, bool bWasSuccessful,
													   const FExternalAuthToken& AuthToken) {
			if (!ModioUISubsystem.IsValid())
			{
				Handler.ExecuteIfBound(false, "ModioUISubsystem is invalid");
				return;
			}
			if (!ModioSubsystem.IsValid())
			{
				Handler.ExecuteIfBound(false, "ModioSubsystem is invalid");
				return;
			}
			if (!bWasSuccessful)
			{
				Handler.ExecuteIfBound(false, "Failed to Get Linked Account Auth Token");
				return;
			}

			FModioEntitlementParams EntParams = FModioEntitlementParams({{"steam_token", AuthToken.TokenString}});
			FOnRefreshUserEntitlementsDelegateFast EntCallback;
			EntCallback.BindLambda([ModioUISubsystem,
									Handler](FModioErrorCode Error,
											 TOptional<FModioEntitlementConsumptionStatusList> StatusList) {
				if (!ModioUISubsystem.IsValid())
				{
					Handler.ExecuteIfBound(false, "ModioUISubsystem is invalid");
					return;
				}

				if (!Error)
				{
					ModioUISubsystem.Get()->RequestWalletBalanceRefresh();
					Handler.ExecuteIfBound(true, "Success");
				}
				else
				{
					Handler.ExecuteIfBound(false, "Failed to Refresh User Entitlements: " + Error.GetErrorMessage());
				}
			});

			ModioSubsystem->RefreshUserEntitlementsAsync(EntParams, EntCallback);
		});

		OnlineSubsystem->GetIdentityInterface()->GetLinkedAccountAuthToken(0, FString(), LinkedAccountAuthCallback);
	}
}