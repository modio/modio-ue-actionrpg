/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include <ActionRPGOnlineModManagementSubsystem.h>

#include "ActionRPGLobbyGameModeBase.generated.h"

/**
 * @nodoc
 */
UCLASS()
class ACTIONRPGMODIO_API AActionRPGLobbyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void InitGameState() override;

protected:
	void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
				  FString& ErrorMessage) override;

private:
	FRPGSessionDelegate OnCreateSessionCompleteDelegate;

	UFUNCTION()
	void OnCreateSessionCompleted(bool bSuccess);
};