/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "ActionRPGCommonTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UI/Interfaces/IModioUIDialog.h"

#ifdef USE_META_PLUGIN
	#include "OVRPlatformCppRequests.h"
	#include "OVRPlatformTypes.h"
#endif

#include "ModioOculusAuthSubsystem.generated.h"

struct FModioErrorCode;
class APlayerController;

UCLASS()
class ACTIONRPGMODIO_API UModioOculusAuthSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void BeginModioAuthentication(EUIMode UIMode);

protected:
	// @TODO: These Show* methods should trigger delegates for the game to hook into to
	// That way this subsystem would not need to hard reference the UI elements
	void UpdateClassSoftReferences(EUIMode UIMode);
	void ShowModBrowser();
	void ShowTermsOfUseDialog();
	void ShowEmailAuth();

	// Callbacks

	void OnAuthenticationComplete(FModioErrorCode ec);
	void OnIsViewerEntitled(bool isEntitled, FString errorMessage);

#ifdef USE_META_PLUGIN
	void OnGetLoggedInUser(bool success, FOvrUserPtr user, FString errorMessage);
	void OnGetUserProof(bool success, FOvrUserProofPtr userProof, FString errorMessage);
	void OnGetAccessToken(bool success, FStringPtr accessToken, FString errorMessage);
#endif

	UFUNCTION()
	void TermsAccepted();

	UFUNCTION()
	void EmailAuthCompleted();

private:
	bool CheckIfSSOReady();
	void AuthenticateModio();
	APlayerController* GetPlayerController() const;
	bool HasUserAcceptedTerms() const;
	bool IsOculusDevice();

	FString TOSDialog;
	FString EmailAuthDialog;
	FString ModBrowser;

#ifdef USE_META_PLUGIN
	FOvrId userId;
	FString userNonce;
	FString userAccessToken;
#endif

	FModioDialogEvent TermsAcceptedEventHandler;
	FModioDialogEvent EmailAuthEventHandler;

	// The bUserHasAcceptedTerms parameter should only be set to true for the specific authentication request made
	// immediately after the user accepts the terms dialog, then revert to false for subsequent requests. Keeping it
	// session-specific prevents stale acceptance data if terms are updated between user sessions.
	bool bHasAcceptedTermsInThisSession = false;
};