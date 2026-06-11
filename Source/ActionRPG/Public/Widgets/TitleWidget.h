/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CommonActivatableWidget.h"
#include "CoreMinimal.h"
#include "TitleWidget.generated.h"

UCLASS()
class ACTIONRPG_API UTitleWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	void OnWidgetAdded(UWidget* Widget, ULocalPlayer* Player);
	void OnWidgetRemoved(UWidget* Widget);
	void UpdateVisibilityOnWidgetChange();

	UPROPERTY()
	TArray<UWidget*> AddedWidgets;
};
