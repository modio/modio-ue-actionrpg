/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPG_UGCProvider.h"

#include "ActionRPGModio.h"

#include "UGC/ModioUGCProvider.h"
#include "UGC/SideLoadUGCProvider.h"

void UActionRPG_UGCProvider::InitializeProvider_Implementation(const FOnUGCProviderInitializedDelegate& Handler)
{
	UGCProviders.Add(NewObject<UModioUGCProvider>(this));
	UGCProviders.Add(NewObject<USideLoadUGCProvider>(this));

	OnUGCProviderInitializedHandler = Handler;

	FOnUGCProviderInitializedDelegate InternalHandler;
	InternalHandler.BindDynamic(this, &UActionRPG_UGCProvider::OnUGCProviderInitialized);

	for (int i = 0; i < UGCProviders.Num(); ++i)
	{
		IUGCProvider::Execute_InitializeProvider(UGCProviders[i], InternalHandler);
	}
}

void UActionRPG_UGCProvider::DeinitializeProvider_Implementation(const FOnUGCProviderDeinitializedDelegate& Handler)
{
	OnUGCProviderDeinitializedHandler = Handler;

	FOnUGCProviderDeinitializedDelegate InternalHandler;
	InternalHandler.BindDynamic(this, &UActionRPG_UGCProvider::OnUGCProviderDeinitialized);

	for (int i = UGCProviders.Num() - 1; i > 0; --i)
	{
		IUGCProvider::Execute_DeinitializeProvider(UGCProviders[i], InternalHandler);
	}
}

bool UActionRPG_UGCProvider::IsProviderEnabled_Implementation()
{
	for (int i = 0; i < UGCProviders.Num(); ++i)
	{
		if (!IUGCProvider::Execute_IsProviderEnabled(UGCProviders[i]))
		{
			return false;
		}
	}
	return true;
}

FModUGCPathMap UActionRPG_UGCProvider::GetInstalledUGCPaths_Implementation()
{
	FModUGCPathMap CombinedPath;
	for (int i = 0; i < UGCProviders.Num(); ++i)
	{
		CombinedPath += IUGCProvider::Execute_GetInstalledUGCPaths(UGCProviders[i]);
	}

	return CombinedPath;
}

void UActionRPG_UGCProvider::OnUGCProviderInitialized(bool bSuccess)
{
	if (bSuccess)
	{
		InitializationCounter++;
		if (InitializationCounter == UGCProviders.Num())
		{
			OnUGCProviderInitializedHandler.ExecuteIfBound(bSuccess);
			OnUGCProviderInitializedHandler.Clear();
		}
	}
	else
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("UGC provider failed to initialize"));
	}
}

void UActionRPG_UGCProvider::OnUGCProviderDeinitialized(bool bSuccess)
{
	if (bSuccess)
	{
		InitializationCounter--;
		if (InitializationCounter == 0)
		{
			OnUGCProviderDeinitializedHandler.ExecuteIfBound(bSuccess);
			OnUGCProviderDeinitializedHandler.Clear();
		}
	}
	else
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("UGC provider failed to deinitialize"));
	}
}