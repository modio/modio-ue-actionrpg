/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"

#include "MapTileWidgetBase.generated.h"

/**
 *
 */
UCLASS()
class ACTIONRPG_API UMapTileWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Init(UObject* InLevel);

protected:
	UPROPERTY(BlueprintReadOnly)
	UObject* Level;
};