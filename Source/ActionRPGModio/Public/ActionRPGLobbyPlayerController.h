/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "ActionRPGOnlineModManagementSubsystem.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "ActionRPGLobbyPlayerController.generated.h"

/**
 * @nodoc
 */
UCLASS()
class ACTIONRPGMODIO_API AActionRPGLobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlayingState() override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerStartGame(FName Map);
};