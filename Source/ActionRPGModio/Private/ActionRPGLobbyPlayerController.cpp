/*
 *  Copyright (C) 2025-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyPlayerController.h"
#include "ModioSubsystem.h"
#include "ActionRPGLobbyGameModeBase.h"
#include "ActionRPGLobbyGameStateBase.h"
#include "ModioLog.h"
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

void AActionRPGLobbyPlayerController::SendModListToServer_Implementation()
{
	if (IsLocalPlayerController())
	{
		if (UModioSubsystem* Modio = GEngine->GetEngineSubsystem<UModioSubsystem>())
		{
			TMap<FModioModID, FModioModCollectionEntry> Mods = Modio->QueryUserInstallations(true);
			TArray<FModioModID> ModList;
			Mods.GenerateKeyArray(ModList);
			UE_LOG(LogOnlineModio, Display, TEXT("Registering %d mods with Server."), ModList.Num());
			RegisterClientModsWithServer(ModList);
		}
	}
}

void AActionRPGLobbyPlayerController::RegisterClientModsWithServer_Implementation(const TArray<FModioModID>& ModList)
{
	if (AActionRPGLobbyGameStateBase* GS = GetWorld()->GetGameState<AActionRPGLobbyGameStateBase>())
	{
		UE_LOG(LogOnlineModio, Display, TEXT("Recieved %d mods for registration from client."), ModList.Num());
		GS->UpdateAllClientModLists(ModList);
	}
}

void AActionRPGLobbyPlayerController::UpdateClientTempMods_Implementation(const TArray<FModioModID>& ModList)
{
	if (IsLocalPlayerController())
	{
		if (UModioSubsystem* Modio = GEngine->GetEngineSubsystem<UModioSubsystem>())
		{
			UE_LOG(LogOnlineModio, Display, TEXT("Recieved Updated Client Temp Mods list form server, totaling %d mods. Updating local temp mod set."), ModList.Num());
			ModManagementEventHandle.BindUObject(this, &AActionRPGLobbyPlayerController::OnModManagementEvent);
			Modio->EnableModManagement(ModManagementEventHandle);
			Modio->AddToTempModSet(ModList);
		}
	}
}

void AActionRPGLobbyPlayerController::OnModManagementEvent(FModioModManagementEvent ModEvent)
{
	UE_LOG(LogOnlineModio, Display, TEXT("Client Mod Managment event for mod %s: %s"), *ModEvent.ID.ToString(),
		   *ModEvent.Status.GetErrorMessage());
}
