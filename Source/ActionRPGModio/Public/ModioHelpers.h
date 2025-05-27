/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "ModioHelpers.generated.h"

enum class EModioPortal : uint8;
enum class EModioEnvironment : uint8;

/**
 *
 */
UCLASS()
class ACTIONRPGMODIO_API UModioHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static bool StringToModioGameEnvironment(const FString& Environment, EModioEnvironment& OutEnvironment);

	static bool StringToModioPortal(const FString& Portal, EModioPortal& OutPortal);
};