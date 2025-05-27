/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "UGC/UGCProvider.h"
#include "UObject/Object.h"

#include "ActionRPG_UGCProvider.generated.h"

/**This class is an aggregate of the examples supplied by the ModioUGC Plugin
 * ModioUGCProvider and ModioUGCProviderSideLoaded. This allows loading UGC which has
 * been authored and uploaded to the mod.io service, as well as side loading UGC
 * from a specific folder for rapid iteration.
 */
UCLASS()
class ACTIONRPGMODIO_API UActionRPG_UGCProvider : public UObject, public IUGCProvider
{
	GENERATED_BODY()

protected:
	//~ Begin IUGCProvider Interface
	virtual void InitializeProvider_Implementation(const FOnUGCProviderInitializedDelegate& Handler) override;
	virtual void DeinitializeProvider_Implementation(const FOnUGCProviderDeinitializedDelegate& Handler) override;
	virtual bool IsProviderEnabled_Implementation() override;
	virtual FModUGCPathMap GetInstalledUGCPaths_Implementation() override;
	//~ End IUGCProvider Interface

	UFUNCTION()
	void OnUGCProviderInitialized(bool bSuccess);

	UFUNCTION()
	void OnUGCProviderDeinitialized(bool bSuccess);

private:
	UPROPERTY()
	TArray<TObjectPtr<UObject>> UGCProviders;

	UPROPERTY()
	FOnUGCProviderInitializedDelegate OnUGCProviderInitializedHandler;

	UPROPERTY()
	FOnUGCProviderDeinitializedDelegate OnUGCProviderDeinitializedHandler;

	UPROPERTY()
	uint8 InitializationCounter = 0;
};