/*
 *  Copyright (C) 2025-2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGLobbyGameModeBase.h"

#include "ActionRPGLobbyGameStateBase.h"
#include "ActionRPGModio.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ModioLog.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"

#include "ModioMultiplayerSubsystem.h"
#include "ActionRPGModioSubsystem.h"
#include "ActionRPGLobbyPlayerController.h"
#include "ActionRPGPlayerState.h"
#include "Types/ModioErrorCode.h"

#include "ActionRPG_UGCProvider.h"
#include "UGC/ModioUGCProvider.h"
#include "UGC/UGCSubsystem.h"

AActionRPGLobbyGameModeBase::AActionRPGLobbyGameModeBase()
	: Super()
{
	GameStateClass = AActionRPGLobbyGameStateBase::StaticClass();
	PlayerControllerClass = AActionRPGLobbyPlayerController::StaticClass();
	PlayerStateClass = AActionRPGPlayerState::StaticClass();
}

void AActionRPGLobbyGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (!IsRunningDedicatedServer())
		return;

	if (UActionRPGModioSubsystem* Modio = GEngine->GetEngineSubsystem<UActionRPGModioSubsystem>())
	{
		Modio->OnServerInit.BindLambda([this](const FModioErrorCode ErrorCode)
		{
			OnModioServerInitComplete(ErrorCode);
		});
		Modio->InitializeModioServices();
	}
}

void AActionRPGLobbyGameModeBase::InitGameState()
{
	Super::InitGameState();

	if (!IsRunningDedicatedServer())
		return;

	AActionRPGLobbyGameStateBase* ArpgGameState = GetGameState<AActionRPGLobbyGameStateBase>();

	if (!ArpgGameState)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get GameState when initialising server."));
		return;
	}

	FString Map = TEXT("ActionRPG_P");
	FParse::Value(FCommandLine::Get(), TEXT("servermap="), Map);

	ArpgGameState->SelectedMap = FName(Map);
	ArpgGameState->AllowPlayerStart = true;
}

void AActionRPGLobbyGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	Cast<AActionRPGLobbyPlayerController>(NewPlayer)->SendModListToServer();
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

void AActionRPGLobbyGameModeBase::OnModioServerInitComplete(FModioErrorCode ErrorCode)
{
#if !UE_SERVER
	return;
#else
	if (ErrorCode)
	{
		UE_LOG(LogOnlineModio, Error, TEXT("Cancelled server launch: Mod.io Service failed to start with error: %s"), *ErrorCode.GetErrorMessage());
		return;
	}

	UActionRPGOnlineModManagementSubsystem* OSS =
		GetWorld()->GetGameInstance()->GetSubsystem<UActionRPGOnlineModManagementSubsystem>();

	if (!OSS)
	{
		UE_LOG(LogOnlineModio, Error, TEXT("Couldn't find Online Subsystem."));
		return;
	}

	UE_LOG(LogOnlineModio, Log, TEXT("Creating server..."));

	FString ServerName = TEXT("DedicatedServer");
	FParse::Value(FCommandLine::Get(), TEXT("servername="), ServerName);

	OnCreateSessionCompleteDelegate.BindUFunction(this, FName("OnCreateSessionCompleted"));
	OSS->CreateSession(ServerName, OnCreateSessionCompleteDelegate);
#endif
}
