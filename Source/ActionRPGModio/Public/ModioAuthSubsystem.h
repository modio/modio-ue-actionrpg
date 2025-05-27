/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "ActionRPGCommonTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Types/ModioInitializeOptions.h"
#include "UI/Interfaces/IModioUIDialog.h"

#include "ModioAuthSubsystem.generated.h"

class USaveGame;
class UModioSaveGame;
struct FModioErrorCode;
struct FExternalAuthToken;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuthenticationComplete);

/**
 *
 */
UCLASS()
class ACTIONRPGMODIO_API UModioAuthSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void BeginModioAuthentication(EUIMode UIMode);

	void AuthenticateModio();

	bool HasUserAcceptedTerms() const;

	FOnAuthenticationComplete OnAuthenticationComplete;

protected:
	// @TODO: These Show* methods should trigger delegates for the game to hook into to
	// That way this subsystem would not need to hard reference the UI elements

	void UpdateClassSoftReferences(EUIMode UIMode);

	void ShowTermsOfUseDialog();

	void ShowEmailAuth();

	void ShowModBrowser();

	// Callbacks
	virtual void OnSubsystemLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
										  const FString& Error);

	virtual void OnGetLinkedAccountTokenComplete(int LocalUserNum, bool bWasSuccessful,
												 const FExternalAuthToken& AuthToken);

	virtual void OnAuthenticationComplete_Internal(FModioErrorCode ec);

	UFUNCTION()
	void EmailAuthCompleted();

	UFUNCTION()
	void TermsAccepted();

private:
	APlayerController* GetPlayerController() const;

	FModioInitializeOptions GetModioInitializeOptions() const;

	TMap<FString, FString> GetExtendedAuthParamsForCurrentPlatform() const;

	FString SaveSlot;
	int32 SaveUserIndex;

	bool ShouldClearSaveDataOnStart();
	bool LoadOrCreateSaveGame();
	UFUNCTION(BlueprintCallable, Category = Save)
	void WriteSaveGame();
	UFUNCTION(BlueprintCallable, Category = Save)
	void ResetSaveGame();

	bool HandleSaveGameLoaded(USaveGame* SaveGameObject);

	// Platforms that will use email auth instead of SSO (mainly for development purposes)
	static TArray<EModioPlatformName> EmailAuthPlatforms;

	// Helper function to check if current platform should use email auth
	static bool ShouldUseEmailAuth();

	FModioDialogEvent TermsAcceptedEventHandler;
	FModioDialogEvent EmailAuthEventHandler;

	UPROPERTY()
	TObjectPtr<UModioSaveGame> SaveGameData;

	FString AuthTokenString;

	FString TOSDialog;
	FString EmailAuthDialog;
	FString ModBrowser;

	// The bUserHasAcceptedTerms parameter should only be set to true for the specific authentication request made
	// immediately after the user accepts the terms dialog, then revert to false for subsequent requests. Keeping it
	// session-specific prevents stale acceptance data if terms are updated between user sessions.
	bool bHasAcceptedTermsInThisSession = false;
};