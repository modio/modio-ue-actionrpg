/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "FindSessionsCallbackProxy.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "ModioSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "ActionRPGOnlineModManagementSubsystem.generated.h"

/**
 * @nodoc
 */
USTRUCT(BlueprintType)
struct FSessionModInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ModName;

	UPROPERTY(BlueprintReadOnly)
	FModioModID ModId;
};

/**
 * @nodoc
 */
USTRUCT(BlueprintType)
struct FSessionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FBlueprintSessionResult Result;

	UPROPERTY(BlueprintReadOnly)
	FString MapName;

	UPROPERTY(BlueprintReadOnly)
	FString GameName;

	UPROPERTY(BlueprintReadOnly)
	TArray<FSessionModInfo> Mods;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FRPGSessionDelegate, bool, Successful);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FRPGOnFindSessionsComplete, const TArray<FSessionResult>&, SessionResults, bool,
								   Successful);
/**
 * @nodoc
 */
UCLASS(DisplayName = "Online Mod Management Subsystem")
class UActionRPGOnlineModManagementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SavePlayerName(FString InPlayerName);

	UFUNCTION(BlueprintCallable)
	void LoadPlayerName(FString& OutPlayerName);

	/**
	 * @docpublic
	 * @brief Create a session, and set the mods required for that session.
	 * @param OnCreateSessionCompleteEvent Blueprint bindable delegate to call when the session creation has finished
	 */
	UFUNCTION(BlueprintCallable)
	void CreateSession(FString GameName, FRPGSessionDelegate OnCreateSessionCompleteEvent);

	/*
	 * @docpublic
	 * @brief Finds all available sessions.
	 * @param OnFindSessionsCompleteEvent Blueprint bindable delegate to run when FindSessions finishes
	 * executing
	 */
	UFUNCTION(BlueprintCallable)
	void FindSessions(FRPGOnFindSessionsComplete OnFindSessionsCompleteEvent);

	/*
	 * @docpublic
	 * @brief Attempt to join SessionResult session.
	 * @param SessionResult The session to try and join
	 * @param OnJoinSessionCompleteEvent Blueprint bindable delegate to call when JoinSession finished
	 * executing
	 */
	UFUNCTION(BlueprintCallable)
	void JoinSession(FSessionResult SessionResult, FRPGSessionDelegate OnJoinSessionCompleteEvent);

	/*
	 * @docpublic
	 * @brief Destroy the current session
	 * @param OnDestroySessionCompleteEvent Blueprint bindable delegate to invoke on destroy completion
	 */
	UFUNCTION(BlueprintCallable)
	void DestroySession(FRPGSessionDelegate OnDestroySessionCompleteEvent);

	/*
	 * @docpublic
	 * @brief Checks if a session currently exsists
	 */
	UFUNCTION(BlueprintCallable)
	bool DoesSessionExist() const;

	/*
	 * @docpublic
	 * @brief End the current session
	 * @param OnEndSessionCompleteEvent Blueprint bindable delegate to be called when EndSession completes
	 */
	UFUNCTION(BlueprintCallable)
	void EndSession(FRPGSessionDelegate OnEndSessionCompleteEvent);

	/*
	 * @docpublic
	 * @brief Start the session
	 * @param OnStartSessionCompleteEvent Blueprint bindable delegate to invoke when StartSession completes
	 */
	UFUNCTION(BlueprintCallable)
	void StartSession(FRPGSessionDelegate OnStartSessionCompleteEvent);

	/*
	 * @docpublic
	 * @brief Try and move the client to the server and its current map
	 * @param SessionResult The server result to move the client to
	 * @returns True if successfully travelled
	 */
	UFUNCTION(BlueprintCallable)
	void TryTravelToCurrentSession(FSessionResult SessionResult);

	/*
	 * @docpublic
	 * @brief Open the chosen level and start listening for connectionss
	 * @param MapString Map to open
	 */
	void OpenLevel(FName InMap);

	/*
	 * @docpublic
	 * @brief Get the mods that are not installed (for any user) on the current clients system
	 * that will need to be installed to join the session
	 * @param SessionResult The session result to test mods against
	 * @returns The required mods
	 */
	UFUNCTION(Blueprintcallable)
	TArray<FModioModID> GetClientMissingMods(FSessionResult SessionResult);

	/*
	 * @docpublic
	 * @brief Create a temporary mod set with all the mods needed to join the server.
	 * @param Result The session result to sync client mods to
	 * @param OnServerModInstallCompleteEvent Blueprint bindable delegate to be called when the server mods are
	 * synced to the client
	 */
	UFUNCTION(BlueprintCallable)
	void SyncServerModsOnClient(FSessionResult Result, FRPGSessionDelegate OnServerModInstallCompleteEvent);

protected:
	/*
	 * @brief The function that is ran when a session create delegate has finished running.
	 * @param SessionName Name of the session
	 * @param bSuccess True if the session was successfully created, otherwise false
	 */
	void OnCreateSessionComplete(FName SessionName, bool bSuccess);

	/*
	 * @brief Function that runs when FindSessions has finished executing.
	 * Here, we iterate through the search results and pull required data from the result.
	 * The result is then wrapped in an FSessionResult, which contains more information (mods and the server name)
	 * @param bSuccess True if FindSessions completed successfully, otherwise false
	 */
	void OnFindSessionsComplete(bool bSuccess);

	/*
	 * @brief Called after JoinSession finishes executing
	 * @param SessionName The session name that was joined
	 * @param Result C++ only Completion Result
	 */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	/*
	 * @brief Called after DestroySession completes
	 * @param SessionName The name of the current session
	 * @param bSuccess True if the session was successfully destroyed
	 */
	void OnDestroySessionComplete(FName SessionName, bool bSuccess);

	/*
	 * @brief Called when EndSession completed
	 * @param SessionName Session name that has ended
	 * @param bSuccess True if EndSession completed successfully
	 */
	void OnEndSessionComplete(FName SessionName, bool bSuccess);

	/*
	 * @brief Called when StartSession is completed
	 * @param SessionName Session name that was started
	 * @param bSuccess True if StartSession completed successfully
	 */
	void OnStartSessionComplete(FName SessionName, bool bSuccess);

	/*
	 * @brief Handle installation events from the TempSet we've made
	 * We check whether the mod event relates to our TempSet, as other events may
	 * come through from pending unrelated mod installation/uninstallation.
	 * If the set is empty (ie, all temp mods are installed) we are finished, and fire the ServerModInstallCompleteEvent
	 * delegate.
	 * @param ModEvent Event to process
	 */
	void OnModManagementEvent(FModioModManagementEvent ModEvent);

private:
	/*
	 * @brief Append the mod id's from the servers User Installations to the sessions settings.
	 * These can then be read on the client when searching for a session.
	 * Session settings only support very basic types, so we send the data as two arrays (Id + Name).
	 * You only need the ModId, but we send the ModName for ease of displaying the name to the client.
	 */
	void AddModsToSessionSettings();

	/*
	 * @brief The reverse of AddModsToSessionSettings. Pulls the session mod data from the SessionSettings and
	 * adds them to the FSessionResult
	 * @param SessionResult
	 */
	void GetModsFromSessionResult(FSessionResult& SessionResult);

	FDelegateHandle FindSessionHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FRPGOnFindSessionsComplete FindSessionsCompleteEvent;

	FDelegateHandle CreateSessionHandle;
	FOnCreateSessionCompleteDelegate SessionCompleteDelegate;
	FRPGSessionDelegate CreateSessionCompleteEvent;

	FDelegateHandle JoinSessionHandle;
	FRPGSessionDelegate JoinSessionCompleteEvent;
	FOnJoinSessionCompleteDelegate SessionJoinDelegate;

	FDelegateHandle DestroySessionHandle;
	FRPGSessionDelegate DestroySessionCompleteEvent;
	FOnDestroySessionCompleteDelegate SessionDestroyDelegate;

	FDelegateHandle EndSessionHandle;
	FRPGSessionDelegate EndSessionCompleteEvent;
	FOnEndSessionCompleteDelegate SessionEndDelegate;

	FDelegateHandle StartSessionHandle;
	FRPGSessionDelegate StartSessionCompleteEvent;
	FOnStartSessionCompleteDelegate StartSessionDelegate;

	TArray<FModioModID> ModsWorkingSet;
	FOnModManagementDelegateFast ModManagementDelegate;
	FRPGSessionDelegate ServerModInstallCompleteEvent;

	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	FString PlayerName = TEXT("Default Player");
};