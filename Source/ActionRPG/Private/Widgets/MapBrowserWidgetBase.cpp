/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "Widgets/MapBrowserWidgetBase.h"

#include "AssetRegistry/AssetRegistryState.h"
#include "Components/PanelWidget.h"
#include "Engine/AssetManager.h"
#include "Misc/EngineVersionComparison.h"
#include "Widgets/MapTileWidgetBase.h"

TArray<TSoftObjectPtr<UObject>> UMapBrowserWidgetBase::FetchMapsList()
{
	TArray<TSoftObjectPtr<UObject>> MapList;

	////////////////////////////////////////////////////////////////////////////
	// Method 1
	// Loading via the Asset Manager
	////////////////////////////////////////////////////////////////////////////
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> OutAssetIDs;
	AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Map"), OutAssetIDs);
	for (FPrimaryAssetId AssetID : OutAssetIDs)
	{
		FAssetData AssetData;
		AssetManager.GetPrimaryAssetData(AssetID, AssetData);

		if (AssetData.IsValid())
		{
			MapList.Add(TSoftObjectPtr<UObject>(AssetData.GetAsset()));
		}
	}
	return MapList;

	////////////////////////////////////////////////////////////////////////////
	// Method 2
	// Loading via the Asset Registry
	////////////////////////////////////////////////////////////////////////////
	/*FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	FARFilter ARFilter;
	TArray<FAssetData> AssetList;
#if UE_VERSION_NEWER_THAN(5, 1, 0)
	ARFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
#else
	ARFilter.ClassNames.Add(UWorld::StaticClass()->GetFName());
#endif
	ARFilter.bIncludeOnlyOnDiskAssets = true;

	FARCompiledFilter CompiledFilter;
	IAssetRegistry::Get()->CompileFilter(ARFilter, CompiledFilter);

	// Query for assets
	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssets(ARFilter, AssetDataArray);

	// Iterate through the found assets and print out their names (or do other logic)
	for (const FAssetData& AssetData : AssetDataArray)
	{
		MapList.Add(TSoftObjectPtr<UObject>(AssetData.GetAsset()));
	}

	return MapList;*/
}