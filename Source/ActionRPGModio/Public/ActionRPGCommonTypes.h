/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "ActionRPGCommonTypes.generated.h"

/**
 * @brief Enum for selecting between Template UI screens and Example UI Screens
 */
UENUM(BlueprintType)
enum EUIMode : uint8
{
	Template,
	Example
};