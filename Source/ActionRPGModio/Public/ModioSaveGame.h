/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "GameFramework/SaveGame.h"

#include "ModioSaveGame.generated.h"

/**
 *
 */
UCLASS()
class ACTIONRPGMODIO_API UModioSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UModioSaveGame();

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;
};