/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "Blueprint/UserWidget.h"
#include "CommonActivatableWidget.h"
#include "Components/TileView.h"

#include "MapBrowserWidgetBase.generated.h"

class UMapTileWidgetBase;

/**
 *
 */
UCLASS()
class ACTIONRPG_API UMapBrowserWidgetBase : public UCommonActivatableWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "UGC")
	TArray<TSoftObjectPtr<UObject>> FetchMapsList();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UGC")
	TSubclassOf<UMapTileWidgetBase> MapTileWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTileView> MapBrowserTileView;
};
