/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ModioAuthSubsystem.h"

#include "ActionRPGModio.h"
#include "ActionRPGModioSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Internationalization/Culture.h"
#include "Kismet/GameplayStatics.h"
#include "Libraries/ModioErrorConditionLibrary.h"
#include "Libraries/ModioPlatformHelpersLibrary.h"
#include "Libraries/ModioSDKLibrary.h"
#include "ModioSaveGame.h"
#include "ModioSubsystem.h"
#include "ModioUISubsystem.h"
#include "OnlineSubsystem.h"
#include "UI/Interfaces/IModioUIDialog.h"

bool UModioAuthSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UModioAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SaveSlot = "ModioSaveGame";
	SaveUserIndex = 0;

	UpdateClassSoftReferences(EUIMode::Template);
	LoadOrCreateSaveGame();

	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		// Initialize the SDK Language
		// Note that we don't want to immediately auth to mod.io and instead, we want to do that when the user
		// interacts with mod.io directly such as opening the mod browser.
		// The SDK itself is intialized only after SSO is complete to ensure correct environment setup
		// UNLESS we are using email auth, in which case we initialize it immediatly

		// Set the language code for the plugin to use
		const EModioLanguage CurrentLanguage = UModioSDKLibrary::GetLanguageCodeFromString(
			FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName());

		ModioSubsystem->SetLanguage(CurrentLanguage);

		// Perform OSS Login at Subsystem Initialize time. This basically guarantees that
		// we will have a token ready for consumption when we browse mods.
		// Note: This may cause issues with token expiration if the game sits in menu for extended periods of time

		const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::GetByPlatform();

		// Set up the OSS and get all the appropriate auth tokens we need.
		if (!ShouldUseEmailAuth())
		{
			// Get the Identity interface from our OnlineSubsystem and perform a Login. This will
			// appropriately auth the user and ensure that we have a valid platform auth token.
			const IOnlineIdentityPtr OnlineIdentity = OnlineSubsystem->GetIdentityInterface();

			OnlineIdentity->AddOnLoginCompleteDelegate_Handle(
				0, FOnLoginCompleteDelegate::CreateUObject(this, &ThisClass::OnSubsystemLoginComplete));
			OnlineIdentity->Login(0, FOnlineAccountCredentials());
		}
	}
}

void UModioAuthSubsystem::BeginModioAuthentication(EUIMode UIMode)
{
	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		TOptional<FModioUser> User = ModioSubsystem->QueryUserProfile();
		UpdateClassSoftReferences(UIMode);
		if (User.IsSet())
		{
			OnAuthenticationComplete_Internal(FModioErrorCode());
		}
		else if (HasUserAcceptedTerms())
		{
			AuthenticateModio();
		}
		else
		{
			ShowTermsOfUseDialog();
		}
	}
}

void UModioAuthSubsystem::AuthenticateModio()
{
	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		// If we have a valid OSS and we have obtained an auth token from the OSS, perform auth
		if (!ShouldUseEmailAuth() && IOnlineSubsystem::GetByPlatform() != nullptr && !AuthTokenString.IsEmpty())
		{
			FModioAuthenticationParams AuthParams;

			AuthParams.AuthToken = AuthTokenString;
			AuthParams.bUserHasAcceptedTerms = bHasAcceptedTermsInThisSession;
			AuthParams.ExtendedParameters = GetExtendedAuthParamsForCurrentPlatform();

			UE_LOG(LogActionRPGModio, Log, TEXT("Authenticating user via OSS"));

			ModioSubsystem->AuthenticateUserExternalAsync(
				AuthParams, UModioPlatformHelpersLibrary::GetDefaultAuthProviderForCurrentPlatform(),
				FOnErrorOnlyDelegateFast::CreateUObject(this, &ThisClass::OnAuthenticationComplete_Internal));
		}
		else
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("Failed to authenticate user via OSS"));

			ShowEmailAuth();
		}
	}
}

bool UModioAuthSubsystem::HasUserAcceptedTerms() const
{
	return bHasAcceptedTermsInThisSession;
}

void UModioAuthSubsystem::UpdateClassSoftReferences(EUIMode UIMode)
{
	if (UIMode == EUIMode::Template)
	{
		TOSDialog = TEXT(
			"/ModioComponentUI/UI/Templates/Default/Dialogs/WBP_ModioDefaultTOSDialog.WBP_ModioDefaultTOSDialog_C");
		EmailAuthDialog = TEXT("/ModioComponentUI/UI/Templates/Default/Dialogs/"
							   "WBP_ModioDefaultEmailAuthDialog.WBP_ModioDefaultEmailAuthDialog_C");
		ModBrowser = TEXT("/ModioComponentUI/UI/Templates/Default/WBP_ModioModBrowser.WBP_ModioModBrowser_C");
	}
	else
	{
		TOSDialog = TEXT(
			"/ModioComponentUI/UI/Templates/Default/Dialogs/WBP_ModioDefaultTOSDialog.WBP_ModioDefaultTOSDialog_C");
		EmailAuthDialog =
			TEXT("/Game/Modding/UGC_Dialogs/WBP_UGC_ModioDefaultEmailAuthDialog.WBP_UGC_ModioDefaultEmailAuthDialog_C");
		ModBrowser = TEXT("/Game/Modding/UGC_Dialogs/WBP_UGC_ModioModBrowser.WBP_UGC_ModioModBrowser_C");
	}
}

void UModioAuthSubsystem::ShowTermsOfUseDialog()
{
	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		// @TODO: Expose this as a config instead of hard referencing it here
		// Or alternatively expose a hook that the game can attach to for this purpose

		const FSoftClassPath TermsOfUseViewReference(TOSDialog);

		if (UClass* ReferencedClass = TermsOfUseViewReference.TryLoadClass<UUserWidget>())
		{
			UUserWidget* TermsWidget = CreateWidget<UUserWidget>(GetPlayerController(), ReferencedClass);
			TermsWidget->AddToViewport();

			if (TermsWidget->Implements<UModioUIDialog>())
			{
				TermsAcceptedEventHandler.BindDynamic(this, &ThisClass::TermsAccepted);
				IModioUIDialog::Execute_AddDialogConfirmedHandler(TermsWidget, TermsAcceptedEventHandler);
			}
		}
	}
}

void UModioAuthSubsystem::ShowEmailAuth()
{
	// @TODO: Expose this as a config instead of hard referencing it here
	// Or alternatively expose a hook that the game can attach to for this purpose
	const FSoftClassPath EmailAuthReference(EmailAuthDialog);

	if (UClass* EmailAuthReferencedClass = EmailAuthReference.TryLoadClass<UUserWidget>())
	{
		UUserWidget* EmailAuthWidget = CreateWidget<UUserWidget>(GetPlayerController(), EmailAuthReferencedClass);
		EmailAuthWidget->AddToViewport();

		if (EmailAuthWidget->Implements<UModioUIDialog>())
		{
			EmailAuthEventHandler.BindDynamic(this, &ThisClass::EmailAuthCompleted);
			IModioUIDialog::Execute_AddDialogConfirmedHandler(EmailAuthWidget, EmailAuthEventHandler);
		}
	}
}

void UModioAuthSubsystem::ShowModBrowser()
{
	if (OnAuthenticationComplete.IsBound())
	{
		OnAuthenticationComplete.Broadcast();
	}
	else
	{
		// @TODO: Expose this as a config instead of hard referencing it here
		if (UClass* ModBrowserWidgetClass = StaticLoadClass(UUserWidget::StaticClass(), nullptr, *ModBrowser))
		{
			UUserWidget* MenuWidget = CreateWidget<UUserWidget>(GetPlayerController(), ModBrowserWidgetClass);
			MenuWidget->AddToViewport();

			// After the browser is added we update the wallet balance, if monetization is enabled.
			if (UModioUISubsystem* Modio = GEngine->GetEngineSubsystem<UModioUISubsystem>())
			{
				if (Modio->IsUGCFeatureEnabled(EModioUIFeatureFlags::Monetization))
				{
					Modio->RequestRefreshEntitlements();
				}
			}
		}
	}
}

void UModioAuthSubsystem::OnSubsystemLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
												   const FString& Error)
{
	// If we've successfully logged in via an OSS, attempt to get a linked account auth token. If successful, this will
	// let us perform SSO.
	if (bWasSuccessful)
	{
		const IOnlineIdentityPtr OnlineIdentity = IOnlineSubsystem::GetByPlatform()->GetIdentityInterface();
		if (OnlineIdentity == nullptr)
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get OnlineIdentity interface for OSS auth"));
			return;
		}
		OnlineIdentity->GetLinkedAccountAuthToken(
			LocalUserNum, FString(),
			IOnlineIdentity::FOnGetLinkedAccountAuthTokenCompleteDelegate::CreateUObject(
				this, &ThisClass::OnGetLinkedAccountTokenComplete));
	}
}

bool Needs64Encoding(const TArray<uint8>& Data)
{
	for (uint8 Byte : Data)
	{
		if (Byte < 32 || Byte > 126)
		{
			return true;
		}
	}
	return false;
}

void UModioAuthSubsystem::OnGetLinkedAccountTokenComplete(int LocalUserNum, bool bWasSuccessful,
														  const FExternalAuthToken& AuthToken)
{
	if (AuthToken.IsValid())
	{
		UE_LOG(LogActionRPGModio, Log, TEXT("Successfully got linked account token in callback"));

		if (AuthToken.HasTokenData())
		{
			if (Needs64Encoding(AuthToken.TokenData))
			{
				AuthTokenString = FBase64::Encode(AuthToken.TokenData);
			}
			else
			{
				AuthTokenString = FString(AuthToken.TokenData.Num(),
										  ANSI_TO_TCHAR(reinterpret_cast<const char*>(AuthToken.TokenData.GetData())));
			}
		}
		else if (AuthToken.HasTokenString())
		{
			AuthTokenString = AuthToken.TokenString;
		}
	}
	else
	{
#if PLATFORM_ANDROID
		FString IdToken, ServerAuthToken;

		IOnlineIdentityPtr OnlineIdentity = IOnlineSubsystem::GetByPlatform()->GetIdentityInterface();
		if (!OnlineIdentity)
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get OnlineIdentity interface for Android auth"));
			return;
		}

		TSharedPtr<FUserOnlineAccount> UserAccount =
			OnlineIdentity->GetUserAccount(*OnlineIdentity->GetUniquePlayerId(0));
		if (!UserAccount.IsValid())
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get UserAccount for Android auth"));
			return;
		}

		UserAccount.Get()->GetAuthAttribute(AUTH_ATTR_ID_TOKEN, IdToken);
		UserAccount.Get()->GetAuthAttribute(AUTH_ATTR_AUTHORIZATION_CODE, ServerAuthToken);

		AuthTokenString = ServerAuthToken;
#else
		UE_LOG(LogActionRPGModio, Log, TEXT("Did not get Linked Account Token. Falling back to GetAuthToken call."));
		IOnlineIdentityPtr OnlineIdentity = IOnlineSubsystem::GetByPlatform()->GetIdentityInterface();
		if (OnlineIdentity.IsValid())
		{
			AuthTokenString = OnlineIdentity->GetAuthToken(0);
		}
#endif
	}
}

void UModioAuthSubsystem::OnAuthenticationComplete_Internal(FModioErrorCode ec)
{
	if (UModioErrorConditionLibrary::ErrorCodeMatches(ec, EModioErrorCondition::UserAlreadyAuthenticatedError))
	{
		// Show Mod Browser
		ShowModBrowser();
	}
	else if (UModioErrorConditionLibrary::ErrorCodeMatches(ec, EModioErrorCondition::UserTermsOfUseError))
	{
		// If the user has not accepted the terms of use, show the dialog
		ShowTermsOfUseDialog();
	}
	else if (ec)
	{
		// Auth has failed for some reason, fall back to the email auth flow as an alternative.
		// In reality, we should only do this if we were attempting an SSO flow and it failed?
		// ShowEmailAuth();
	}
	else
	{
		ShowModBrowser();
	}
}

void UModioAuthSubsystem::EmailAuthCompleted()
{
	// Show Mod Browser
	ShowModBrowser();
}

void UModioAuthSubsystem::TermsAccepted()
{
	bHasAcceptedTermsInThisSession = true;
}

APlayerController* UModioAuthSubsystem::GetPlayerController() const
{
	return GetLocalPlayer()->GetPlayerController(GetWorld());
}

TMap<FString, FString> UModioAuthSubsystem::GetExtendedAuthParamsForCurrentPlatform() const
{
	// @TODO: Abstract this out so that we can keep NDA code out of this subsystem for an eventual public release
	return TMap<FString, FString>();
}

TArray<EModioPlatformName> UModioAuthSubsystem::EmailAuthPlatforms = {
	EModioPlatformName::Android,
	EModioPlatformName::Switch,
};

bool UModioAuthSubsystem::ShouldUseEmailAuth()
{
	if (EmailAuthPlatforms.Contains(UModioPlatformHelpersLibrary::GetCurrentPlatform()))
	{
		return true;
	}

	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::GetByPlatform();
	if (OnlineSubsystem == nullptr)
	{
		return true;
	}

	const IOnlineIdentityPtr OnlineIdentity = OnlineSubsystem->GetIdentityInterface();
	if (OnlineIdentity == nullptr)
	{
		return true;
	}

	return false;
}

bool UModioAuthSubsystem::ShouldClearSaveDataOnStart()
{
	bool bClearSaveData = false;
	FString Value;
	if (FParse::Value(FCommandLine::Get(), TEXT("clearsavedata"), Value))
	{
		bClearSaveData = true;
	}
	UE_LOG(LogActionRPGModio, Display, TEXT("Clear save data on start?: %hs"), bClearSaveData ? "true" : "false");
	return bClearSaveData;
}

bool UModioAuthSubsystem::LoadOrCreateSaveGame()
{
	if (ShouldClearSaveDataOnStart())
	{
		return HandleSaveGameLoaded(nullptr);
	}

	UModioSaveGame* LoadedSave = nullptr;

	if (UGameplayStatics::DoesSaveGameExist(SaveSlot, SaveUserIndex))
	{
		LoadedSave = Cast<UModioSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, SaveUserIndex));
	}

	return HandleSaveGameLoaded(LoadedSave);
}

void UModioAuthSubsystem::WriteSaveGame()
{
	UGameplayStatics::SaveGameToSlot(SaveGameData, SaveSlot, SaveUserIndex);
}

void UModioAuthSubsystem::ResetSaveGame()
{
	// Call handle function with no loaded save, this will reset the data
	HandleSaveGameLoaded(nullptr);
}

bool UModioAuthSubsystem::HandleSaveGameLoaded(USaveGame* SaveGameObject)
{
	bool bLoaded = false;

	// Replace current save, old object will GC out
	SaveGameData = Cast<UModioSaveGame>(SaveGameObject);

	if (SaveGameData)
	{
		bLoaded = true;
	}
	else
	{
		// This creates it on demand
		SaveGameData = Cast<UModioSaveGame>(UGameplayStatics::CreateSaveGameObject(UModioSaveGame::StaticClass()));
	}

	return bLoaded;
}