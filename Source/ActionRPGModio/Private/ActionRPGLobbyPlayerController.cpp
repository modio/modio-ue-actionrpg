/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyPlayerController.h"

#include "ActionRPGLobbyGameModeBase.h"
#include "ActionRPGPlayerState.h"

void AActionRPGLobbyPlayerController::BeginPlayingState()
{
	Super::BeginPlayingState();

	if (IsLocalPlayerController())
	{
		// Tell the Server my name!
		if (AActionRPGPlayerState* PS = Cast<AActionRPGPlayerState>(PlayerState))
		{
			if (UActionRPGOnlineModManagementSubsystem* OSS =
					GetWorld()->GetGameInstance()->GetSubsystem<UActionRPGOnlineModManagementSubsystem>())
			{
				FString PlayerName;
				OSS->LoadPlayerName(PlayerName);
				PS->ServerUpdatePlayerName(PlayerName);
			}
		}
	}
}

void AActionRPGLobbyPlayerController::ServerStartGame_Implementation(FName Map)
{
	if (UActionRPGOnlineModManagementSubsystem* OSS =
			GetWorld()->GetGameInstance()->GetSubsystem<UActionRPGOnlineModManagementSubsystem>())
	{
		OSS->OpenLevel(Map);
	}
}