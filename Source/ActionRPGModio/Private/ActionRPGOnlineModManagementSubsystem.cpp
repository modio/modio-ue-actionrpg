/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGOnlineModManagementSubsystem.h"

#include "ActionRPGModio.h"
#include "Engine/Level.h"

void UActionRPGOnlineModManagementSubsystem::SavePlayerName(FString InPlayerName)
{
	if (InPlayerName.IsEmpty())
		return;

	PlayerName = InPlayerName;
}

void UActionRPGOnlineModManagementSubsystem::LoadPlayerName(FString& OutPlayerName)
{
	OutPlayerName = PlayerName;
}

void UActionRPGOnlineModManagementSubsystem::CreateSession(FString GameName,
														   FRPGSessionDelegate OnCreateSessionCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		OnCreateSessionCompleteEvent.ExecuteIfBound(false);
		return;
	}

	CreateSessionCompleteEvent = OnCreateSessionCompleteEvent;

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPrivateConnections = 0;
	SessionSettings->NumPublicConnections = 10;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bIsLANMatch = true;

	// Example of setting extra data to read on the clients
	// This is only used in this instance to show the map and game name in the session browser
	SessionSettings->Set(FName("MapName"), GetWorld()->GetMapName(), EOnlineDataAdvertisementType::ViaOnlineService);

	if (GameName.IsEmpty())
	{
		GameName = TEXT("Default Game Name");
	}

	SessionSettings->Set(FName("GameName"), GameName, EOnlineDataAdvertisementType::ViaOnlineService);

	AddModsToSessionSettings();

	SessionCompleteDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnCreateSessionComplete);
	CreateSessionHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(SessionCompleteDelegate);

	if (!IsRunningDedicatedServer())
	{
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession,
											 *SessionSettings))
		{
			OnCreateSessionCompleteEvent.ExecuteIfBound(false);
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
		}
	}
	else
	{
		if (!SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings))
		{
			OnCreateSessionCompleteEvent.ExecuteIfBound(false);
			SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
		}
	}
}

void UActionRPGOnlineModManagementSubsystem::AddModsToSessionSettings()
{
	TMap<FModioModID, FModioModCollectionEntry> InstalledMods =
		GEngine->GetEngineSubsystem<UModioSubsystem>()->QueryUserInstallations(false);

	SessionSettings->Set(FName("NumMods"), InstalledMods.Num(), EOnlineDataAdvertisementType::ViaOnlineService);
	int i = 0;
	for (auto& Mod : InstalledMods)
	{
		FName IdKey = FName(*FString::Printf(TEXT("ModId_[%d]"), i));
		SessionSettings->Set(IdKey, Mod.Value.GetID().ToString(), EOnlineDataAdvertisementType::ViaOnlineService);
		FName NameKey = FName(*FString::Printf(TEXT("ModName_[%d]"), i++));
		SessionSettings->Set(NameKey, Mod.Value.GetModProfile().ProfileName,
							 EOnlineDataAdvertisementType::ViaOnlineService);
	}
}

void UActionRPGOnlineModManagementSubsystem::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (SessionInterface)
	{
		if (!IsRunningDedicatedServer())
		{
			// Register the player that created the session
			// If you don't do this, when searching for a session - the result will contain 0/Max players, instead of
			// 1/Max players. All players eventually need to be registered
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			SessionInterface->RegisterPlayer(SessionName, *LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId(),
											 false);
		}

		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionHandle);
	}

	CreateSessionCompleteEvent.ExecuteIfBound(bSuccess);
	CreateSessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::FindSessions(FRPGOnFindSessionsComplete OnFindSessionsCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		OnFindSessionsCompleteEvent.ExecuteIfBound(TArray<FSessionResult>(), false);
		return;
	}

	FindSessionsCompleteEvent = OnFindSessionsCompleteEvent;

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = 10;
	LastSessionSearch->bIsLanQuery = true;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	FindSessionsCompleteDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnFindSessionsComplete);

	FindSessionHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		OnFindSessionsCompleteEvent.ExecuteIfBound(TArray<FSessionResult>(), false);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionHandle);
	}
}

void UActionRPGOnlineModManagementSubsystem::OnFindSessionsComplete(bool bSuccess)
{
	TArray<FSessionResult> SessionResults;

	for (auto& Result : LastSessionSearch->SearchResults)
	{
		FBlueprintSessionResult BPResult;
		BPResult.OnlineResult = Result;

		FSessionResult SessionResult;
		SessionResult.Result = std::move(BPResult);

		Result.Session.SessionSettings.Get("MapName", SessionResult.MapName);
		Result.Session.SessionSettings.Get("GameName", SessionResult.GameName);

		GetModsFromSessionResult(SessionResult);

		SessionResults.Add(std::move(SessionResult));
	}

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionHandle);
	}

	FindSessionsCompleteEvent.ExecuteIfBound(SessionResults, bSuccess);
	FindSessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::GetModsFromSessionResult(FSessionResult& SessionResult)
{
	int32 NumMods;
	SessionResult.Result.OnlineResult.Session.SessionSettings.Get("NumMods", NumMods);

	for (int i = 0; i < NumMods; ++i)
	{
		FSessionModInfo Info;
		FName IdKey = FName(*FString::Printf(TEXT("ModId_[%d]"), i));

		FString IDString;
		SessionResult.Result.OnlineResult.Session.SessionSettings.Get(IdKey, IDString);
		Info.ModId = FModioModID(FCString::Atoi64(*IDString));

		FName NameKey = FName(*FString::Printf(TEXT("ModName_[%d]"), i));
		SessionResult.Result.OnlineResult.Session.SessionSettings.Get(NameKey, Info.ModName);

		SessionResult.Mods.Add(std::move(Info));
	}
}

void UActionRPGOnlineModManagementSubsystem::JoinSession(FSessionResult SessionResult,
														 FRPGSessionDelegate OnJoinSessionCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		OnJoinSessionCompleteEvent.ExecuteIfBound(false);
		return;
	}

	JoinSessionCompleteEvent = OnJoinSessionCompleteEvent;

	SessionJoinDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnJoinSessionComplete);

	JoinSessionHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(SessionJoinDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession,
									   SessionResult.Result.OnlineResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
		OnJoinSessionCompleteEvent.ExecuteIfBound(false);
	}
}

void UActionRPGOnlineModManagementSubsystem::OnJoinSessionComplete(FName SessionName,
																   EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionHandle);
	}

	// return true if sucess
	JoinSessionCompleteEvent.ExecuteIfBound(Result == EOnJoinSessionCompleteResult::Success);
	JoinSessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::DestroySession(FRPGSessionDelegate OnDestroySessionCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		OnDestroySessionCompleteEvent.ExecuteIfBound(false);
		return;
	}

	DestroySessionCompleteEvent = OnDestroySessionCompleteEvent;

	SessionDestroyDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnDestroySessionComplete);

	DestroySessionHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(SessionDestroyDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);

		OnDestroySessionCompleteEvent.ExecuteIfBound(false);
	}
}

bool UActionRPGOnlineModManagementSubsystem::DoesSessionExist() const
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		return false;
	}

	return SessionInterface->GetNamedSession(NAME_GameSession) != nullptr;
}

void UActionRPGOnlineModManagementSubsystem::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionHandle);
	}

	DestroySessionCompleteEvent.ExecuteIfBound(true);
	DestroySessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::EndSession(FRPGSessionDelegate OnEndSessionCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface)
	{
		OnEndSessionCompleteEvent.ExecuteIfBound(false);
		return;
	}

	EndSessionCompleteEvent = OnEndSessionCompleteEvent;

	SessionEndDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnEndSessionComplete);

	EndSessionHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(SessionEndDelegate);

	if (!SessionInterface->EndSession(NAME_GameSession))
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionHandle);

		OnEndSessionCompleteEvent.ExecuteIfBound(false);
	}
}

void UActionRPGOnlineModManagementSubsystem::OnEndSessionComplete(FName SessionName, bool bSuccess)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionHandle);
	}

	EndSessionCompleteEvent.ExecuteIfBound(true);
	EndSessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::StartSession(FRPGSessionDelegate OnStartSessionCompleteEvent)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		return;
	}

	StartSessionCompleteEvent = OnStartSessionCompleteEvent;

	StartSessionDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnStartSessionComplete);

	StartSessionHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionDelegate);

	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionHandle);

		OnStartSessionCompleteEvent.ExecuteIfBound(false);
	}
}

void UActionRPGOnlineModManagementSubsystem::OnStartSessionComplete(FName SessionName, bool bSuccess)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (SessionInterface)
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionHandle);
	}

	StartSessionCompleteEvent.ExecuteIfBound(bSuccess);
	StartSessionHandle.Reset();
}

void UActionRPGOnlineModManagementSubsystem::TryTravelToCurrentSession(FSessionResult SessionResult)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionResult.Result.OnlineResult, NAME_GamePort, ConnectString))
	{
		return;
	}

	FURL NewURL(nullptr, *ConnectString, ETravelType::TRAVEL_Absolute);
	FString BrowseError;
	if (GEngine->Browse(GEngine->GetWorldContextFromWorldChecked(GetWorld()), NewURL, BrowseError) ==
		EBrowseReturnVal::Failure)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to start browse: %s"), *BrowseError);
	}
}

void UActionRPGOnlineModManagementSubsystem::OpenLevel(FName InMap)
{
	const FString URL = FString::Printf(TEXT("%s?listen"), *InMap.ToString());
	GetWorld()->ServerTravel(URL, true);
}

TArray<FModioModID> UActionRPGOnlineModManagementSubsystem::GetClientMissingMods(FSessionResult SessionResult)
{
	TMap<FModioModID, FModioModCollectionEntry> InstalledMods =
		GEngine->GetEngineSubsystem<UModioSubsystem>()->QuerySystemInstallations();

	TMap<FModioModID, FModioModCollectionEntry> InstalledTempMods =
		GEngine->GetEngineSubsystem<UModioSubsystem>()->QueryTempModSet();

	InstalledMods.Append(InstalledTempMods);

	TArray<FModioModID> MissingIds;
	for (auto& ServerMod : SessionResult.Mods)
	{
		bool bFound = false;
		for (auto& ClientMod : InstalledMods)
		{
			if (ServerMod.ModId == ClientMod.Key)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			MissingIds.Add(ServerMod.ModId);
		}
	}

	return MissingIds;
}

void UActionRPGOnlineModManagementSubsystem::SyncServerModsOnClient(FSessionResult Result,
																	FRPGSessionDelegate OnServerModInstallCompleteEvent)
{
	if (Result.Mods.Num() <= 0)
	{
		OnServerModInstallCompleteEvent.ExecuteIfBound(true);
		return;
	}

	UModioSubsystem* ModIoSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();

	if (!ModIoSubsystem)
	{
		OnServerModInstallCompleteEvent.ExecuteIfBound(false);
		return;
	}

	ServerModInstallCompleteEvent = OnServerModInstallCompleteEvent;

	TArray<FModioModID> TempSet;
	for (auto& Mod : Result.Mods)
	{
		TempSet.Add(Mod.ModId);
	}

	ModsWorkingSet = GetClientMissingMods(Result);

	// Only need to install missing mods
	if (!ModsWorkingSet.IsEmpty())
	{
		ModIoSubsystem->EnableModManagement(ModManagementDelegate);
		ModManagementDelegate.BindUObject(this, &UActionRPGOnlineModManagementSubsystem::OnModManagementEvent);
	}

	ModIoSubsystem->InitTempModSet(TempSet);
}

void UActionRPGOnlineModManagementSubsystem::OnModManagementEvent(FModioModManagementEvent ModEvent)
{
	if (!ModEvent.Status && ModEvent.Event == EModioModManagementEventType::Installed)
	{
		if (ModsWorkingSet.Contains(ModEvent.ID))
		{
			ModsWorkingSet.Remove(ModEvent.ID);
		}

		// All required mods installed
		if (ModsWorkingSet.IsEmpty())
		{
			UModioSubsystem* ModIoSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
			ServerModInstallCompleteEvent.ExecuteIfBound(true);
			ModIoSubsystem->DisableModManagement();
		}
	}
	else if (ModEvent.Status)
	{
		if (ModsWorkingSet.Contains(ModEvent.ID))
		{
			UModioSubsystem* ModIoSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
			// Failed to install one of the mods
			ServerModInstallCompleteEvent.ExecuteIfBound(false);
			ModIoSubsystem->DisableModManagement();
		}
	}
}