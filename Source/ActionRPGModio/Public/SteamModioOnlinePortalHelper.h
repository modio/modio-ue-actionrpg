/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreTypes.h"
#include "Interfaces/IModioPortalInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"

#include "SteamModioOnlinePortalHelper.generated.h"

UCLASS()
class ACTIONRPGMODIO_API USteamModioOnlinePortalImplementation : public UObject, public IModioPortalInterface
{
	GENERATED_BODY()

protected:
	virtual void NativeRequestAuthToken(const TMap<FString, FString>& Params,
										const FAuthTokenRequestedDelegateFast& Callback) override
	{
		IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::GetByPlatform();

		IOnlineIdentity::FOnGetLinkedAccountAuthTokenCompleteDelegate LinkedAccountAuthCallback =
			IOnlineIdentity::FOnGetLinkedAccountAuthTokenCompleteDelegate::CreateLambda(
				[Callback](int32 LocalUserNum, bool bWasSuccessful, const FExternalAuthToken& AuthToken) {
					Callback.ExecuteIfBound(AuthToken.TokenString);
				});

		OnlineSubsystem->GetIdentityInterface()->GetLinkedAccountAuthToken(0, TEXT(""),
																		   MoveTemp(LinkedAccountAuthCallback));
	}

	virtual void NativeRequestEntitlementParams(const TMap<FString, FString>& Params,
												const FEntitlementParamsRequestedDelegateFast& Callback) override
	{
		NativeRequestAuthToken(
			Params, FAuthTokenRequestedDelegateFast::CreateLambda([this, Callback](const FString& AuthToken) {
				Callback.ExecuteIfBound(
					FModioEntitlementParams({{IModioPortalInterface::Execute_GetAuthKey(this), AuthToken}}));
			}));
	}

	virtual FString NativeGetAuthKey() override
	{
		return TEXT("steam_token");
	}
};
