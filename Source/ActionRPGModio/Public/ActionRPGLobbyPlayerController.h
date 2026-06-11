/*
 *  Copyright (C) 2025-2026 mod.io Pty Ltd. <https://mod.io>
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

	
	UFUNCTION(Client, Reliable)
	void SendModListToServer();

	UFUNCTION(Server, Reliable)
	void RegisterClientModsWithServer(const TArray<FModioModID>& ModList);

	UFUNCTION(Client, Reliable)
	void UpdateClientTempMods(const TArray<FModioModID>& ModList);

	void OnModManagementEvent(FModioModManagementEvent ModEvent);
	FOnModManagementDelegateFast ModManagementEventHandle;
};