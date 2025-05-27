/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "Subsystems/EngineSubsystem.h"
#include "UI/EventHandlers/IModioUIEntitlementRefreshEventReceiver.h"

#include "ModioEntitlementSubsystem.generated.h"

class IOnlineSubsystem;
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnUserEntitlementsRefreshedDelegate, bool, bSuccess, const FString&, Message);

/**
 *
 */
UCLASS()
class UModioEntitlementSubsystem : public UEngineSubsystem, public IModioUIEntitlementRefreshEventReceiver
{
	GENERATED_BODY()

public:
	// Entitlement Refresh Interface
	virtual void NativeOnEntitlementRefreshEvent() override;
	// End Entitlement Refresh Interface

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Mod.io Entitlement Subsystem")
	void RefreshUserEntitlements();

	UFUNCTION(BlueprintCallable, Category = "Mod.io Entitlement Subsystem")
	void RefreshUserEntitlementsWithHandler(const FOnUserEntitlementsRefreshedDelegate& Handler);

private:
	void RefreshEntitlementsSteam(IOnlineSubsystem* OnlineSubsystem, TWeakObjectPtr<UModioSubsystem> ModioSubsystem,
								  TWeakObjectPtr<UModioUISubsystem> ModioUISubsystem,
								  const FOnUserEntitlementsRefreshedDelegate& Handler);
};