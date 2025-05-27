/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ActionRPGModio.h"

#include "ActionRPGModioSubsystem.h"
#include "Libraries/ModioSDKLibrary.h"
#include "ModioEntitlementSubsystem.h"

#if PLATFORM_WINDOWS
THIRD_PARTY_INCLUDES_START
// @TODO: Fix include error here
// #include "steam/steam_api.h"
THIRD_PARTY_INCLUDES_END
#endif

static FAutoConsoleCommand CmdInitializeSDK(
	TEXT("Modio.Initialize"), TEXT("Initialize Modio SDK, Run Background thread"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem)
		{
			FModioInitializeOptions Options = UModioSDKLibrary::GetProjectInitializeOptions();
			Options.LocalSessionIdentifier = "LocalSession";

			ModioSubsystem->InitializeAsync(
				Options, FOnErrorOnlyDelegateFast::CreateLambda([](FModioErrorCode ErrorCode) {
					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();

					if (ErrorCode == false)
					{
						PC->ClientMessage("InitializeAsync Successful");
					}
					else
					{
						PC->ClientMessage(
							FString::Printf(TEXT("InitializeAsync Failed, Error : %s"), *ErrorCode.GetErrorMessage()));
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdShutdownSDK(
	TEXT("Modio.Shutdown"), TEXT("Shutdown the SDK"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem)
		{
			ModioSubsystem->ShutdownAsync(FOnErrorOnlyDelegateFast::CreateLambda([](FModioErrorCode ErrorCode) {
				APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();

				if (ErrorCode == false)
				{
					PC->ClientMessage("ShutdownAsync Successful");
				}
				else
				{
					PC->ClientMessage(
						FString::Printf(TEXT("ShutdownAsync Failed, Error : %s"), *ErrorCode.GetErrorMessage()));
				}
			}));
		}
	}));

static FAutoConsoleCommand CmdLoadFile(TEXT("Modio.LoadFile"), TEXT("Load a texture from a download dir"),
									   FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
										   TArray<uint8> Data;
										   FFileHelper::LoadFileToArray(Data, *Args[0], FILEREAD_None);
									   }));

static FAutoConsoleCommand CmdEnableModManagement(
	TEXT("Modio.EnableModManagement"), TEXT("Enable ModManagement"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem)
		{
			ModioSubsystem->EnableModManagement(
				FOnModManagementDelegateFast::CreateLambda([](FModioModManagementEvent Event) {
					FString NameEvent =
						StaticEnum<EModioModManagementEventType>()->GetNameStringByValue((int) Event.Event);

					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
					PC->ClientMessage(FString::Printf(TEXT("ModManagement : ModId : %s , Event : %s , Status: %s"),
													  *Event.ID.ToString(), *NameEvent,
													  *Event.Status.GetErrorMessage()));
				}));
		}
	}));

static FAutoConsoleCommand CmdDisableModManagement(
	TEXT("Modio.DisableModManagement"), TEXT("Disable ModManagement"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem)
		{
			ModioSubsystem->DisableModManagement();
			APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
			PC->ClientMessage("ModManagement Disabled");
		}
	}));

static FAutoConsoleCommand CmdRequestAuthCode(
	TEXT("Modio.RequestEmailAuthCodeAsync"), TEXT("Arg : Email"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 1)
		{
			FModioEmailAddress EmailAddress = FModioEmailAddress(*Args[0]);

			ModioSubsystem->RequestEmailAuthCodeAsync(
				EmailAddress, FOnErrorOnlyDelegateFast::CreateLambda([EmailAddress](FModioErrorCode ErrorCode) {
					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();

					if (ErrorCode == false)
					{
						PC->ClientMessage("RequestEmailAuthCodeAsync Successful");
					}
					else
					{
						PC->ClientMessage(FString::Printf(TEXT("RequestEmailAuthCodeAsync Failed, Error : %s"),
														  *ErrorCode.GetErrorMessage()));
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdSAuthEmail(
	TEXT("Modio.AuthenticateUserEmailAsync"), TEXT("Arg : Code"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 1)
		{
			FModioEmailAuthCode AuthCode = FModioEmailAuthCode(*Args[0]);

			ModioSubsystem->AuthenticateUserEmailAsync(
				AuthCode, FOnErrorOnlyDelegateFast::CreateLambda([AuthCode](FModioErrorCode ErrorCode) {
					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();

					if (ErrorCode == false)
					{
						PC->ClientMessage("AuthEmail Successful");
					}
					else
					{
						PC->ClientMessage(
							FString::Printf(TEXT("AuthEmail Failed, Error : %s"), *ErrorCode.GetErrorMessage()));
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdSubmitMod(
	TEXT("Modio.SubmitNewModAsync"), TEXT("Arg : Name, Description, PathToLogoFile, Summary"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 4)
		{
			FModioModCreationHandle Handle = ModioSubsystem->GetModCreationHandle();

			FString pathToLogo = FPaths::RootDir() / *Args[2];
			pathToLogo = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*pathToLogo);

			APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
			PC->ClientMessage(FString::Printf(TEXT("Using PathToLogo : %s"), *pathToLogo));

			FModioCreateModParams Params;
			Params.Name = *Args[0];
			Params.Description = *Args[1];
			Params.PathToLogoFile = pathToLogo;
			Params.Summary = *Args[3];

			ModioSubsystem->SubmitNewModAsync(
				Handle, Params,
				FOnSubmitNewModDelegateFast::CreateLambda([PC, Params](FModioErrorCode ErrorCode,
																	   TOptional<FModioModID> ModId) {
					if (ErrorCode == false)
					{
						PC->ClientMessage(FString::Printf(TEXT("Mod Submitted Successfully : %s"), *ModId->ToString()));
					}
					else
					{
						PC->ClientMessage(
							FString::Printf(TEXT("SubmitMod Failed, Error : %s"), *ErrorCode.GetErrorMessage()));
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdSubmitModChange(
	TEXT("Modio.SubmitModChangesAsync"), TEXT("Arg : ModId, Name, PathToLogoFile"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 3)
		{
			FString strModId = *Args[0];
			FModioModID ModID = FModioModID(FCString::Atoi(*strModId));

			FString pathToLogo = FPaths::RootDir() / *Args[2];
			pathToLogo = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*pathToLogo);

			APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
			PC->ClientMessage(FString::Printf(TEXT("Using PathToLogo : %s"), *pathToLogo));

			FModioEditModParams Params;
			Params.Name = *Args[1];
			Params.LogoPath = *pathToLogo;

			ModioSubsystem->SubmitModChangesAsync(
				ModID, Params,
				FOnGetModInfoDelegateFast::CreateLambda(
					[PC, Params](FModioErrorCode ErrorCode, TOptional<FModioModInfo> ModInfo) {
						if (!ErrorCode && ModInfo.IsSet())
						{
							PC->ClientMessage(
								FString::Printf(TEXT("Mod Change Successful : %s"), *ModInfo->ModId.ToString()));
						}
						else
						{
							PC->ClientMessage(
								FString::Printf(TEXT("SubmitMod Failed, Error : %s"), *ErrorCode.GetErrorMessage()));
						}
					}));
		}
	}));

static FAutoConsoleCommand CmdNewFileMod(
	TEXT("Modio.SubmitNewModFileForMod"), TEXT("Arg : ModId, PathToFolder"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 2)
		{
			FString strModId = *Args[0];
			FModioModID ModID = FModioModID(FCString::Atoi(*strModId));

			FString pathToModRoot = FPaths::RootDir() / *Args[1];
			pathToModRoot = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*pathToModRoot);

			APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
			PC->ClientMessage(FString::Printf(TEXT("Using PathToMod : %s"), *pathToModRoot));

			FModioCreateModFileParams Params;
			Params.PathToModRootDirectory = pathToModRoot;
			Params.bSetAsActiveRelease = true;

			ModioSubsystem->SubmitNewModFileForMod(ModID, Params);
		}
	}));

static FAutoConsoleCommand CmdListAllMods(
	TEXT("Modio.ListAllModsAsync"), TEXT("List All Mod, log Name and ModId"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && GEngine->GameViewport)
		{
			ModioSubsystem->ListAllModsAsync(
				FModioFilterParams(),
				FOnListAllModsDelegateFast::CreateLambda(
					[](FModioErrorCode ErrorCode, TOptional<FModioModInfoList> ModList) {
						APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
						if (ErrorCode == false)
						{
							PC->ClientMessage("ListAllMods Successfully");

							for (FModioModInfo Info : ModList.GetValue().GetRawList())
							{
								PC->ClientMessage(FString::Printf(TEXT("Mod - ID : %s , Name : %s"),
																  *Info.ModId.ToString(), *Info.ProfileName));
							}
						}
						else
						{
							PC->ClientMessage(FString::Printf(TEXT("ListAllModsAsync Failed, Error : %s"),
															  *ErrorCode.GetErrorMessage()));
						}
					}));
		}
	}));

static FAutoConsoleCommand CmdGetUserWalletBalance(
	TEXT("Modio.GetUserWalletBalanceAsync"), TEXT("Get the users Wallet balance"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && GEngine->GameViewport)
		{
			ModioSubsystem->GetUserWalletBalanceAsync(FOnGetUserWalletBalanceDelegateFast::CreateLambda(
				[](FModioErrorCode ErrorCode, TOptional<uint64> WalletBalance) {
					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
					if (ErrorCode == false)
					{
						UE_LOG(LogActionRPGModio, Log, TEXT("GetUserWalletBalance successful; balance is %llu"),
							   WalletBalance.GetValue());
					}
					else
					{
						UE_LOG(LogActionRPGModio, Log, TEXT("GetUserWalletBalance Failed; Error %s"),
							   *ErrorCode.GetErrorMessage());
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdPurchaseMod(
	TEXT("Modio.PurchaseModAsync"), TEXT("Purchase a mod; requires ModID and ExpectedPrice"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && Args.Num() == 2 && GEngine->GameViewport)
		{
			FString strModId = *Args[0];
			FModioModID ModID = FModioModID(FCString::Atoi(*strModId));

			FString strModPrice = *Args[1];
			uint64_t Price = uint64_t(FCString::Atoi(*strModPrice));

			ModioSubsystem->PurchaseModAsync(
				ModID, Price,
				FOnPurchaseModDelegateFast::CreateLambda(
					[](FModioErrorCode ErrorCode, TOptional<FModioTransactionRecord> Transaction) {
						APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
						if (ErrorCode == false)
						{
							if (ErrorCode == false)
							{
								UE_LOG(LogActionRPGModio, Log,
									   TEXT("PurchaseModAsync successful; Updated wallet balance is %llu"),
									   Transaction.GetValue().UpdatedUserWalletBalance.Underlying);
							}
							else
							{
								UE_LOG(LogActionRPGModio, Log, TEXT("PurchaseModAsync Failed; Error %s"),
									   *ErrorCode.GetErrorMessage());
							}
						}
					}));
		}
	}));

static FAutoConsoleCommand CmdFetchUserPurchases(
	TEXT("Modio.FetchUserPurchasesAsync"), TEXT("Fetch the users purchases"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && GEngine->GameViewport)
		{
			ModioSubsystem->FetchUserPurchasesAsync(
				FOnFetchUserPurchasesDelegateFast::CreateLambda([](FModioErrorCode ErrorCode) {
					APlayerController* PC = GEngine->GameViewport->GetWorld()->GetFirstPlayerController();
					if (ErrorCode == false)
					{
						if (ErrorCode == false)
						{
							UE_LOG(LogActionRPGModio, Log,
								   TEXT("FetchUserPurchasesAsync successful; query using QueryUserPurchases"));
						}
						else
						{
							UE_LOG(LogActionRPGModio, Log, TEXT("FetchUserPurchasesAsync Failed; Error %s"),
								   *ErrorCode.GetErrorMessage());
						}
					}
				}));
		}
	}));

static FAutoConsoleCommand CmdQueryUserPurchases(
	TEXT("Modio.QueryUserPurchasedMods"), TEXT("Query all of the user's purchases"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && GEngine->GameViewport)
		{
			auto PurchasedMods = ModioSubsystem->QueryUserPurchasedMods();

			for (auto Info : PurchasedMods)
			{
				UE_LOG(LogActionRPGModio, Log, TEXT("Mod ID: %s, Name: %s"), *Info.Key.ToString(),
					   *Info.Value.ProfileName);
			}
		}
	}));

static FAutoConsoleCommand CmdClearUserData(
	TEXT("Modio.ClearUserData"), TEXT("Clear user data"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioSubsystem* ModioSubsystem = GEngine->GetEngineSubsystem<UModioSubsystem>();
		if (ModioSubsystem && GEngine->GameViewport)
		{
			ModioSubsystem->ClearUserDataAsync(
				FOnErrorOnlyDelegateFast::CreateLambda([](FModioErrorCode ErrorCode) {}));
		}
	}));

static FAutoConsoleCommand CmdCrash(TEXT("Modio.Crash"), TEXT("Crash the game"),
									FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
										check(0);
									}));

static FAutoConsoleCommand CmdAddSteamTokenPack(
	TEXT("Modio.AddSteamTokenPack"),
	TEXT("Adds a single token pack to the currently authenticated steam user (does not consume it)\r\nTakes two "
		 "numeric arguments, optionally: Item ID and Quantity, which default to 101 and 1 respectivly, like "
		 "so:\r\nModio.AddSteamTokenPack 101 1"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		int32 Result = 0;
		if (Args.Num() == 0)
		{
			UActionRPGModioSubsystem::AddItemToSteamInventory(Result);
		}
		else if (Args.Num() == 1 && Args[0].IsNumeric())
		{
			UActionRPGModioSubsystem::AddItemToSteamInventory(Result, FCString::Atoi(*Args[0]));
		}
		else if (Args.Num() == 2 && Args[0].IsNumeric() && Args[1].IsNumeric())
		{
			UActionRPGModioSubsystem::AddItemToSteamInventory(Result, FCString::Atoi(*Args[0]),
															  FCString::Atoi(*Args[1]));
		}
		else
		{
			UE_LOG(LogActionRPGModio, Warning,
				   TEXT("Too many, or non-numeric arguments handed to Modio.AddSteamTokenPack"));
		}
		UE_LOG(LogTemp, Warning, TEXT("Generate Items result: %d"), Result);
	}));

static FAutoConsoleCommand CmdRefreshEntitlements(
	TEXT("Modio.RefreshEntitlements"), TEXT("Refresh Entitlements"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) {
		UModioEntitlementSubsystem* ModioEntitlementSubsystem =
			GEngine->GetEngineSubsystem<UModioEntitlementSubsystem>();
		if (ModioEntitlementSubsystem)
		{
			ModioEntitlementSubsystem->RefreshUserEntitlements();
		}
	}));

IMPLEMENT_MODULE(FActionRPGModioModule, ActionRPGModio)

/** Logging definitions */
DEFINE_LOG_CATEGORY(LogActionRPGModio);