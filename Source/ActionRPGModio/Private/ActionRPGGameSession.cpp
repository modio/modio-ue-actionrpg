/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGGameSession.h"
#include "ActionRPGLobbyGameModeBase.h"

void AActionRPGGameSession::NotifyLogout(const APlayerController* PC)
{
	Super::NotifyLogout(PC);

	CurrentPlayers--;

	if (CurrentPlayers == 0 && IsRunningDedicatedServer())
	{
		if (HasAuthority())
		{
			if (UActionRPGOnlineModManagementSubsystem* OSS =
					GetWorld()->GetGameInstance()->GetSubsystem<UActionRPGOnlineModManagementSubsystem>())
			{
				// Change map even if failed
				OSS->OpenLevel(TEXT("ActionRPG_Lobby"));
			}
		}
	}
}

void AActionRPGGameSession::NotifyLogout(FName InSessionName, const FUniqueNetIdRepl& UniqueId) 
{
	Super::NotifyLogout(InSessionName, UniqueId);
}

void AActionRPGGameSession::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	CurrentPlayers++;
}