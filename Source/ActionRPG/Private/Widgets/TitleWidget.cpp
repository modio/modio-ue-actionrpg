/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "Widgets/TitleWidget.h"
#include "Blueprint/GameViewportSubsystem.h"

void UTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameViewportSubsystem* Subsystem = UGameViewportSubsystem::Get(GetWorld());
	if (Subsystem)
	{
		Subsystem->OnWidgetAdded.AddUObject(this, &UTitleWidget::OnWidgetAdded);
		Subsystem->OnWidgetRemoved.AddUObject(this, &UTitleWidget::OnWidgetRemoved);
	}
}

void UTitleWidget::NativeDestruct()
{
	Super::NativeDestruct();

	UGameViewportSubsystem* Subsystem = UGameViewportSubsystem::Get(GetWorld());
	if (Subsystem)
	{
		Subsystem->OnWidgetAdded.RemoveAll(this);
	}
}

void UTitleWidget::OnWidgetAdded(UWidget* Widget, ULocalPlayer* Player)
{
	if (Widget == this)
	{
		return;
	}
	AddedWidgets.AddUnique(Widget);
	UpdateVisibilityOnWidgetChange();
}

void UTitleWidget::OnWidgetRemoved(UWidget* Widget)
{
	AddedWidgets.Remove(Widget);
	UpdateVisibilityOnWidgetChange();
}

void UTitleWidget::UpdateVisibilityOnWidgetChange()
{
	// If there are no widgets in the viewport above this menu then be visible and allow focus.
	// Otherwise become invisible to prevent stealing focus from the widgets above this one.
	if (AddedWidgets.IsEmpty())
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
