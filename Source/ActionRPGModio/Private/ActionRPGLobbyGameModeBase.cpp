/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyGameModeBase.h"

#include "ActionRPGLobbyGameStateBase.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ModioLog.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

void AActionRPGLobbyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!IsRunningDedicatedServer())
		return;

	UActionRPGOnlineModManagementSubsystem* OSS =
		GetWorld()->GetGameInstance()->GetSubsystem<UActionRPGOnlineModManagementSubsystem>();

	if (!OSS)
	{
		ErrorMessage = TEXT("Couldn't find Online Subsystem");
		return;
	}

	UE_LOG(LogOnlineModio, Log, TEXT("Creating server..."));

	FString ServerName = TEXT("DedicatedServer");
	FParse::Value(FCommandLine::Get(), TEXT("servername="), ServerName);

	OnCreateSessionCompleteDelegate.BindUFunction(this, FName("OnCreateSessionCompleted"));
	OSS->CreateSession(ServerName, OnCreateSessionCompleteDelegate);
}

void AActionRPGLobbyGameModeBase::InitGameState()
{
	Super::InitGameState();

	if (!IsRunningDedicatedServer())
		return;

	AActionRPGLobbyGameStateBase* ArpgGameState = GetGameState<AActionRPGLobbyGameStateBase>();

	FString Map = TEXT("ActionRPG_P");
	FParse::Value(FCommandLine::Get(), TEXT("servermap="), Map);

	ArpgGameState->SelectedMap = FName(Map);
	ArpgGameState->AllowPlayerStart = true;
}

void AActionRPGLobbyGameModeBase::PreLogin(const FString& Options, const FString& Address,
										   const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface->RegisterPlayer(NAME_GameSession, *UniqueId, false))
	{
		FString MyErrorMessage = TEXT("Couldn't register player.");
		FGameModeEvents::GameModePreLoginEvent.Broadcast(this, UniqueId, MyErrorMessage);
		return;
	}

	AGameModeBase::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void AActionRPGLobbyGameModeBase::OnCreateSessionCompleted(bool bSuccess)
{
	OnCreateSessionCompleteDelegate.Unbind();

	if (bSuccess)
	{
		UE_LOG(LogOnlineModio, Log, TEXT("Created server."));
	}
	else
	{
		UE_LOG(LogOnlineModio, Log, TEXT("Failed to create server."));
	}
}