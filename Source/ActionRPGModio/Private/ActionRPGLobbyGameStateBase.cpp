/*
 *  Copyright (C) 2025-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyGameStateBase.h"

#include "ModioMultiplayerSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "ActionRPGPlayerState.h"
#include "ActionRPGLobbyPlayerController.h"
#include "ModioLog.h"
#include <Net/UnrealNetwork.h>
#include "Async/Async.h"

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

void AActionRPGLobbyGameStateBase::UpdateAllClientModLists_Implementation(const TArray<FModioModID>& ModList)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		if (UModioMultiplayerSubsystem* MP = GEngine->GetEngineSubsystem<UModioMultiplayerSubsystem>())
		{
			MP->RegisterClientModsWithServerAsync(
				ModList, FAddClientModsDelegateFast::CreateLambda([this, ModList](FModioErrorCode Ec, TSet<FModioModID> Mods)
				{
					AsyncTask(ENamedThreads::GameThread, [this, Ec, Mods, ModList]()
					{
						if (!Ec)
						{
							UE_LOG(LogOnlineModio, Display,
								   TEXT("Registered %d mods with the server, client mod list totals %d mods. Updating "
										"all Clients."),
								   ModList.Num(), Mods.Num());
							for (APlayerState* Player : PlayerArray)
							{
								Cast<AActionRPGLobbyPlayerController>(
									Cast<AActionRPGPlayerState>(Player)->GetPlayerController())
									->UpdateClientTempMods(Mods.Array());
							}
						}
					});
				}));
		}
	}
}

void AActionRPGLobbyGameStateBase::OnRep_SelectedMap()
{
	MapUpdated.Broadcast(SelectedMap);
}

void AActionRPGLobbyGameStateBase::OnRep_AllowPlayerStart() {}