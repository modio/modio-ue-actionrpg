/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"

#include "ActionRPGGameSession.generated.h"
/**
 * @nodoc
 */
UCLASS()
class ACTIONRPGMODIO_API AActionRPGGameSession : public AGameSession
{
	GENERATED_BODY()

	void NotifyLogout(const APlayerController* PC) override;
	void NotifyLogout(FName InSessionName, const FUniqueNetIdRepl& UniqueId) override;
	void PostLogin(APlayerController* NewPlayer) override;

private:
	int32 CurrentPlayers = 0;
};