/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ModioOculusAuthSubsystem.h"

#include "ActionRPGModio.h"
#include "Blueprint/UserWidget.h"
#include "Libraries/ModioErrorConditionLibrary.h"
#include "ModioSubsystem.h"
#include "ModioUISubsystem.h"

#ifdef USE_META_PLUGIN
	#include "OVRPlatformSubsystem.h"
	#include "OVRPlatformUtils.h"
#endif

#if PLATFORM_ANDROID
	#include "Android/AndroidApplication.h"
	#include "Android/AndroidJNI.h"
	#include <android_native_app_glue.h>
#endif

bool UModioOculusAuthSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UModioOculusAuthSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
#ifdef USE_META_PLUGIN
	Collection.InitializeDependency<UOvrPlatformSubsystem>();

	UE_LOG(LogActionRPGModio, Log, TEXT("UModioOculusAuthSubsystem::Initialize"));

	Super::Initialize(Collection);
	UpdateClassSoftReferences(EUIMode::Template);

	if (!IsOculusDevice())
	{
		// Not a Meta device, don't go any further
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("GameInstance is null in Initialize()!"));
		return;
	}

	auto ovrSubsystem = GameInstance->GetSubsystem<UOvrPlatformSubsystem>();

	if (ovrSubsystem == nullptr)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get OvrPlatformSubsystem."));
		return;
	}

	ovrSubsystem->StartMessagePump();

	OvrPlatform_Entitlement_GetIsViewerEntitled_Delegate EntitlementDelegate =
		OvrPlatform_Entitlement_GetIsViewerEntitled_Delegate::CreateUObject(
			this, &UModioOculusAuthSubsystem::OnIsViewerEntitled);
	OvrPlatform_Entitlement_GetIsViewerEntitled(GameInstance, MoveTemp(EntitlementDelegate));
#endif
}

void UModioOculusAuthSubsystem::OnIsViewerEntitled(bool isEntitled, FString errorMessage)
{
#ifdef USE_META_PLUGIN
	if (isEntitled)
	{
		UE_LOG(LogActionRPGModio, Display, TEXT("Entitlement check successful."));

		UGameInstance* GameInstance = GetGameInstance();
		if (!GameInstance)
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("GameInstance is null in Initialize()!"));
			return;
		}

		// call OVR Get Logged In User
		OvrPlatform_User_GetLoggedInUser_Delegate LoggedInUserDelegate =
			OvrPlatform_User_GetLoggedInUser_Delegate::CreateUObject(this,
																	 &UModioOculusAuthSubsystem::OnGetLoggedInUser);
		OvrPlatform_User_GetLoggedInUser(GameInstance, MoveTemp(LoggedInUserDelegate));

		// Get the nonce from OVR
		OvrPlatform_User_GetUserProof_Delegate UserProofDelegate =
			OvrPlatform_User_GetUserProof_Delegate::CreateUObject(this, &UModioOculusAuthSubsystem::OnGetUserProof);
		OvrPlatform_User_GetUserProof(GameInstance, MoveTemp(UserProofDelegate));

		// Get access token from OVR
		OvrPlatform_User_GetAccessToken_Delegate AccessTokenDelegate =
			OvrPlatform_User_GetAccessToken_Delegate::CreateUObject(this, &UModioOculusAuthSubsystem::OnGetAccessToken);
		OvrPlatform_User_GetAccessToken(GameInstance, MoveTemp(AccessTokenDelegate));
	}
	else
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Entitlement check failed: %s."), *errorMessage);
	}
#endif
}

#ifdef USE_META_PLUGIN
void UModioOculusAuthSubsystem::OnGetLoggedInUser(bool success, FOvrUserPtr user, FString errorMessage)
{
	UE_LOG(LogActionRPGModio, Display, TEXT("UModioOculusAuthSubsystem::OnGetLoggedInUser"));
	if (!success || !user.IsValid())
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get the logged in user: %s"), *errorMessage);
		return;
	}

	userId = user->ID;
	FString userIdStr = UOvrPlatformUtilsLibrary::Conv_OvrIdToString(userId);
	UE_LOG(LogActionRPGModio, Display, TEXT("Meta User Id: %s"), *userIdStr);
}

void UModioOculusAuthSubsystem::OnGetUserProof(bool success, FOvrUserProofPtr userProof, FString errorMessage)
{
	UE_LOG(LogActionRPGModio, Display, TEXT("UModioOculusAuthSubsystem::OnGetUserProof"));
	if (!success || !userProof.IsValid())
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get user proof: %s"), *errorMessage);
		return;
	}

	userNonce = userProof->Nonce;
	UE_LOG(LogActionRPGModio, Display, TEXT("Nonce: %s"), *userNonce);
}

void UModioOculusAuthSubsystem::OnGetAccessToken(bool success, FStringPtr accessToken, FString errorMessage)
{
	UE_LOG(LogActionRPGModio, Display, TEXT("UModioOculusAuthSubsystem::OnGetAccessToken"));
	if (!success || !accessToken.IsValid())
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Failed to get access token: %s"), *errorMessage);
		return;
	}

	userAccessToken = *accessToken;
	UE_LOG(LogActionRPGModio, Display, TEXT("Access Token: %s"), *userAccessToken);
}
#endif

bool UModioOculusAuthSubsystem::CheckIfSSOReady()
{
#ifdef USE_META_PLUGIN
	UE_LOG(LogActionRPGModio, Display, TEXT("FOnlineIdentityMeta::CheckIfSSOReady"));

	if (userId > 0 && !userNonce.IsEmpty() && !userAccessToken.IsEmpty())
	{
		UE_LOG(LogActionRPGModio, Display, TEXT("SSO is ready"));
		return true;
	}
#endif

	return false;
}

void UModioOculusAuthSubsystem::BeginModioAuthentication(EUIMode UIMode)
{
	UE_LOG(LogActionRPGModio, Log, TEXT("UModioOculusAuthSubsystem::BeginModioAuthentication"));

	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		UpdateClassSoftReferences(UIMode);
		TOptional<FModioUser> User = ModioSubsystem->QueryUserProfile();
		if (User.IsSet())
		{
			OnAuthenticationComplete(FModioErrorCode());
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

void UModioOculusAuthSubsystem::AuthenticateModio()
{
	UE_LOG(LogActionRPGModio, Log, TEXT("UModioOculusAuthSubsystem::AuthenticateModio"));

	if (UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>())
	{
		// If we have a valid OSS and we have obtained an auth token from the OSS, perform auth
		if (CheckIfSSOReady())
		{
#ifdef USE_META_PLUGIN
			FModioAuthenticationParams AuthParams;

			TMap<FString, FString> extendedParams = {
				{TEXT("device"), TEXT("quest")},
				{TEXT("user_id"), UOvrPlatformUtilsLibrary::Conv_OvrIdToString(userId)},
				{TEXT("nonce"), userNonce}};

			AuthParams.AuthToken = userAccessToken;
			AuthParams.bUserHasAcceptedTerms = bHasAcceptedTermsInThisSession;
			AuthParams.ExtendedParameters = extendedParams;

			UE_LOG(LogActionRPGModio, Log, TEXT("Authenticating user via OSS"));

			ModioSubsystem->AuthenticateUserExternalAsync(
				AuthParams, EModioAuthenticationProvider::Oculus,
				FOnErrorOnlyDelegateFast::CreateUObject(this, &ThisClass::OnAuthenticationComplete));
#endif
		}
		else
		{
			UE_LOG(LogActionRPGModio, Error, TEXT("Failed to authenticate user via OSS"));

			ShowEmailAuth();
		}
	}
}

void UModioOculusAuthSubsystem::OnAuthenticationComplete(FModioErrorCode ec)
{
	UE_LOG(LogActionRPGModio, Log, TEXT("UModioOculusAuthSubsystem::OnAuthenticationComplete"));

	if (UModioErrorConditionLibrary::ErrorCodeMatches(ec, EModioErrorCondition::UserAlreadyAuthenticatedError))
	{
		UE_LOG(LogActionRPGModio, Log, TEXT("User is already authenticated"));

		// Show Mod Browser
		ShowModBrowser();
	}
	else if (UModioErrorConditionLibrary::ErrorCodeMatches(ec, EModioErrorCondition::UserTermsOfUseError))
	{
		UE_LOG(LogActionRPGModio, Warning, TEXT("User has not accepted terms of use"));
		// If the user has not accepted the terms of use, show the dialog
		ShowTermsOfUseDialog();
	}
	else if (ec)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("Authentication failed: %s"), *ec.GetErrorMessage());
		// Auth has failed for some reason, fall back to the email auth flow as an alternative.
		// In reality, we should only do this if we were attempting an SSO flow and it failed?
		// ShowEmailAuth();
	}
	else
	{
		UE_LOG(LogActionRPGModio, Log, TEXT("Authentication successful!"));

		ShowModBrowser();
	}
}

void UModioOculusAuthSubsystem::UpdateClassSoftReferences(EUIMode UIMode)
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

void UModioOculusAuthSubsystem::ShowModBrowser()
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

APlayerController* UModioOculusAuthSubsystem::GetPlayerController() const
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogActionRPGModio, Error, TEXT("GameInstance is null in GetPlayerController()!"));
		return nullptr;
	}

	return GameInstance->GetFirstLocalPlayerController();
}

void UModioOculusAuthSubsystem::ShowTermsOfUseDialog()
{
	UE_LOG(LogActionRPGModio, Log, TEXT("UModioOculusAuthSubsystem::ShowTermsOfUseDialog"));
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

bool UModioOculusAuthSubsystem::HasUserAcceptedTerms() const
{
	return bHasAcceptedTermsInThisSession;
}

void UModioOculusAuthSubsystem::TermsAccepted()
{
	bHasAcceptedTermsInThisSession = true;
}

void UModioOculusAuthSubsystem::ShowEmailAuth()
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

void UModioOculusAuthSubsystem::EmailAuthCompleted()
{
	// Show Mod Browser
	ShowModBrowser();
}

bool UModioOculusAuthSubsystem::IsOculusDevice()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jclass BuildClass = Env->FindClass("android/os/Build");
		if (!BuildClass)
		{
			return false;
		}

		jfieldID ManufacturerField = Env->GetStaticFieldID(BuildClass, "MANUFACTURER", "Ljava/lang/String;");
		if (!ManufacturerField)
		{
			return false;
		}

		jstring ManufacturerJString = (jstring) Env->GetStaticObjectField(BuildClass, ManufacturerField);
		const char* ManufacturerCString = Env->GetStringUTFChars(ManufacturerJString, 0);

		FString Manufacturer(ManufacturerCString);
		Env->ReleaseStringUTFChars(ManufacturerJString, ManufacturerCString);
		Env->DeleteLocalRef(ManufacturerJString);
		Env->DeleteLocalRef(BuildClass);

		// Check if the manufacturer is "Oculus" or "Meta"
		return Manufacturer.Contains(TEXT("Oculus")) || Manufacturer.Contains(TEXT("Meta"));
	}
#endif

	return false; // Non-Android platforms are not Oculus devices
}