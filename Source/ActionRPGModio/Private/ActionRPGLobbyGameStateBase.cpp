/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyGameStateBase.h"

#include <Net/UnrealNetwork.h>

void AActionRPGLobbyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AActionRPGLobbyGameStateBase, SelectedMap);
	DOREPLIFETIME(AActionRPGLobbyGameStateBase, AllowPlayerStart);
}

void AActionRPGLobbyGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	PlayerJoined.Broadcast(PlayerState);
}

void AActionRPGLobbyGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	PlayerLeft.Broadcast(PlayerState);
}

void AActionRPGLobbyGameStateBase::ChangeMap_Implementation(FName InMap)
{
	SelectedMap = InMap;

	// Make sure we call broadcast on Listen Servers
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_SelectedMap();
	}
}

void AActionRPGLobbyGameStateBase::OnRep_SelectedMap()
{
	MapUpdated.Broadcast(SelectedMap);
}

void AActionRPGLobbyGameStateBase::OnRep_AllowPlayerStart() {}