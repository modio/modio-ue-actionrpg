/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGPlayerState.h"

#include "Net/UnrealNetwork.h"
#include <ActionRPGOnlineModManagementSubsystem.h>

void AActionRPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AActionRPGPlayerState, CustomPlayerName);
	DOREPLIFETIME(AActionRPGPlayerState, bIsReady);
}

bool AActionRPGPlayerState::ServerUpdatePlayerName_Validate(const FString& NewPlayerName)
{
	return !NewPlayerName.IsEmpty() && NewPlayerName.Len() <= 32;
}

void AActionRPGPlayerState::ServerUpdatePlayerName_Implementation(const FString& NewPlayerName)
{
	CustomPlayerName = NewPlayerName;

	// Make sure we call broadcast on Listen Servers
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_CustomPlayerName();
	}
}

void AActionRPGPlayerState::ServerUpdateReadyStatus_Implementation(const bool bInIsReady)
{
	bIsReady = bInIsReady;

	// Make sure we call broadcast on Listen Servers
	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_ReadyStatus();
	}
}

void AActionRPGPlayerState::OnRep_CustomPlayerName()
{
	PlayerNameUpdated.Broadcast(CustomPlayerName);
}

void AActionRPGPlayerState::OnRep_ReadyStatus()
{
	PlayerReadyStatusUpdated.Broadcast(bIsReady);
}