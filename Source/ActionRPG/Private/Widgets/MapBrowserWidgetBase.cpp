/*
 *  Copyright (C) 2025-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "Widgets/MapBrowserWidgetBase.h"

#include "AssetRegistry/AssetRegistryState.h"
#include "Components/PanelWidget.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "Misc/EngineVersionComparison.h"
#include "Widgets/MapTileWidgetBase.h"

TArray<TSoftObjectPtr<UObject>> UMapBrowserWidgetBase::FetchMapsList()
{
	////////////////////////////////////////////////////////////////////////////
	// Method 1
	// Loading via the Asset Manager
	////////////////////////////////////////////////////////////////////////////
	UE_LOG(LogTemp, Log, TEXT("Finding maps via Asset Manager"));
	TArray<TSoftObjectPtr<UObject>> AssetManagerMapList;
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> OutAssetIDs;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Map"), OutAssetIDs);

	for (const FPrimaryAssetId& AssetID : OutAssetIDs)
	{
		FAssetData AssetData;
		AssetManager.GetPrimaryAssetData(AssetID, AssetData);

		if (AssetData.IsValid())
		{
			// Use AssetData.AssetName instead of loading the asset
			UE_LOG(LogTemp, Log, TEXT("    -> Found map: %s"), *AssetData.AssetName.ToString());

			// Create TSoftObjectPtr from the asset path WITHOUT loading it
			FSoftObjectPath AssetPath = AssetData.GetSoftObjectPath();
			AssetManagerMapList.Add(TSoftObjectPtr<UObject>(AssetPath));
		}
	}
	////////////////////////////////////////////////////////////////////////////
	// Method 2
	// Loading via the Asset Registry
	////////////////////////////////////////////////////////////////////////////
	UE_LOG(LogTemp, Log, TEXT("Finding maps via Asset Registry"));
	TArray<TSoftObjectPtr<UObject>> AssetRegistryMapList;
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(AssetRegistryConstants::ModuleName);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter ARFilter;
#if UE_VERSION_NEWER_THAN(5, 1, 0)
	ARFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
#else
	ARFilter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#endif
	ARFilter.bIncludeOnlyOnDiskAssets = true;

	// Query for assets
	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssets(ARFilter, AssetDataArray);

	for (const FAssetData& AssetData : AssetDataArray)
	{
		if (AssetData.IsValid())
		{
			// Use AssetData.AssetName instead of loading the asset
			UE_LOG(LogTemp, Log, TEXT("    -> Found map: %s"), *AssetData.AssetName.ToString());

			// Create TSoftObjectPtr from the asset path WITHOUT loading it
			FSoftObjectPath AssetPath = AssetData.GetSoftObjectPath();
			AssetRegistryMapList.Add(TSoftObjectPtr<UObject>(AssetPath));
		}
	}

	if (LoadMethod == EMapLoadMethod::AssetManager)
	{
		return AssetManagerMapList;
	}
	else
	{
		return AssetRegistryMapList;
	}
}