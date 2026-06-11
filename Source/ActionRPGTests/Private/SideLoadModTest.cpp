#include "Misc/AutomationTest.h"
#include "UGC/SideLoadUGCProvider.h"
#include "UGC/UGCSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Guid.h"
#include "UObject/UnrealType.h"
#include "ActionRPGTests.h"
#include "Tests/AutomationCommon.h"
#include "GameFramework/GameStateBase.h"
#include "UObject/GarbageCollection.h"

#if WITH_EDITOR

DEFINE_LOG_CATEGORY_STATIC(LogActionRpgAutomationTest, Log, All);

namespace
{
	bool GetSubsystemProvider(UUGCSubsystem* UGCSubsystem, TScriptInterface<IUGCProvider>& OutProvider)
	{
		if (!UGCSubsystem)
		{
			return false;
		}

		const FInterfaceProperty* ProviderProperty =
			FindFProperty<FInterfaceProperty>(UUGCSubsystem::StaticClass(), TEXT("UGCProvider"));
		if (!ProviderProperty)
		{
			return false;
		}

		const FScriptInterface* ProviderValue = ProviderProperty->ContainerPtrToValuePtr<FScriptInterface>(UGCSubsystem);
		if (!ProviderValue || !ProviderValue->GetObject())
		{
			return false;
		}

		OutProvider.SetObject(ProviderValue->GetObject());
		OutProvider.SetInterface(Cast<IUGCProvider>(ProviderValue->GetObject()));
		return OutProvider.GetObject() != nullptr;
	}
}

// A stateful test to cover the full lifecycle
class FSideLoadModTestBase : public FAutomationTestBase
{
public:
	bool bLoaded = false;
	bool bUnloaded = false;
	bool bProviderInitFailed = false;

	virtual bool RunTestLoop();

	FSideLoadModTestBase(const FString& InName, const bool bInComplexTask)
		: FAutomationTestBase(InName, bInComplexTask)
	{
	}

	virtual bool RunTest(const FString& Parameters) override;

private:
	enum class ETestState
	{
		InitialCleanup,
		CopyMod,
		InitializeProvider,
		VerifyLoaded,
		LoadMap,
		WaitForMapLoad,
		CloseMap,
		UnloadMod,
		VerifyUnloaded,
		FinalCleanup,
		Done
	};

	ETestState CurrentState = ETestState::InitialCleanup;
	double StateStartSeconds = 0.0;

	FString TestFixturePath;
	FString SideloadPath;
	FString WorkingModPath;
	FString ExpectedPluginName;
	FString ExpectedAssetPath;
	FString ConflictingProjectPluginPath;
	FString ConflictingProjectPluginDisabledPath;
	int32 BaselinePackageCount = 0;
	TScriptInterface<IUGCProvider> OriginalProvider;
	UObject* OriginalProviderObject = nullptr;

	void AdvanceState(ETestState NextState)
	{
		CurrentState = NextState;
		StateStartSeconds = FPlatformTime::Seconds();
	}

	bool HasStateTimedOut(double TimeoutSeconds) const
	{
		return (FPlatformTime::Seconds() - StateStartSeconds) >= TimeoutSeconds;
	}

	bool FailAndQueueCleanup(const FString& ErrorMessage)
	{
		AddError(ErrorMessage);
		AdvanceState(ETestState::FinalCleanup);
		return false;
	}

	bool GetMapsFromAssetManager(int32& OutCount)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		TArray<FPrimaryAssetId> MapAssetIds;
		AssetManager.GetPrimaryAssetIdList(FPrimaryAssetType("Map"), MapAssetIds);
		OutCount = MapAssetIds.Num();
		return true;
	}

	int32 MapCountBeforeMod = 0;

	USideLoadTestHelper* HelperObject = nullptr;
};

void USideLoadTestHelper::OnProviderInitialized(bool bSuccess)
{
	if (!bSuccess && TestInstance)
	{
		TestInstance->AddError(TEXT("Failed to initialize SideLoadUGCProvider"));
		TestInstance->bProviderInitFailed = true;
	}

	if (bSuccess && TestInstance)
	{
		UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Provider Initialized. Refreshing UGC."));
		UUGCSubsystem* UGCSubsystem = GEngine->GetEngineSubsystem<UUGCSubsystem>();

		if (!UGCSubsystem)
		{
			TestInstance->AddError(TEXT("UUGCSubsystem missing during provider initialization callback."));
			TestInstance->bProviderInitFailed = true;
			return;
		}

		UGCSubsystem->RefreshUGC();
		TestInstance->bLoaded = true;
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FRunSideLoadModTestCommand, FSideLoadModTestBase*, TestInstance);

bool FRunSideLoadModTestCommand::Update()
{
	if (TestInstance)
	{
		return TestInstance->RunTestLoop();
	}
	return true;
}

bool FSideLoadModTestBase::RunTest(const FString& Parameters)
{
	UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Starting side load lifecycle test."));

	AdvanceState(ETestState::InitialCleanup);
	bLoaded = false;
	bUnloaded = false;
	bProviderInitFailed = false;
	ExpectedPluginName.Reset();
	ExpectedAssetPath.Reset();
	ConflictingProjectPluginPath.Reset();
	ConflictingProjectPluginDisabledPath.Reset();
	BaselinePackageCount = 0;
	OriginalProvider = TScriptInterface<IUGCProvider>();
	OriginalProviderObject = nullptr;

	// Suppress expected warnings from duplicate plugin discovery during reload cycle.
	AddExpectedError(TEXT("Another plugin of the same name was found"), EAutomationExpectedErrorFlags::Contains, 0);

	// Suppress unrelated Tolgee localization errors (network requests to tolgee.io may fail in CI/test environments).
	AddExpectedError(TEXT("LogTolgee"), EAutomationExpectedErrorFlags::Contains, 0);

	// <ProjectDir>\ModioTestFixtures
	TestFixturePath = FPaths::ProjectDir() / TEXT("ModioTestFixtures");
	// <ProjectDir>\Modio
	SideloadPath = FPaths::ProjectDir() / TEXT("Modio");
	WorkingModPath = SideloadPath /
		FString::Printf(TEXT("__AutomationSideLoadFixture_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits));

	if (!HelperObject)
	{
		HelperObject = NewObject<USideLoadTestHelper>();
		HelperObject->AddToRoot(); // Prevent GC
	}
	HelperObject->TestInstance = this;

	// Wait for standard tick before test starts doing latent things
	ADD_LATENT_AUTOMATION_COMMAND(FRunSideLoadModTestCommand(this));
	return true;
}

bool FSideLoadModTestBase::RunTestLoop()
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileManager& FileManager = IFileManager::Get();

	UUGCSubsystem* UGCSubsystem = GEngine->GetEngineSubsystem<UUGCSubsystem>();
	if (!UGCSubsystem && CurrentState != ETestState::FinalCleanup && CurrentState != ETestState::Done)
	{
		return FailAndQueueCleanup(TEXT("UUGCSubsystem is missing."));
	}

	switch (CurrentState)
	{
		case ETestState::InitialCleanup:
		{
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Preparing sideload test paths. Root=%s, WorkingDir=%s"),
				   *SideloadPath, *WorkingModPath);
			if (!PlatformFile.DirectoryExists(*SideloadPath) && !PlatformFile.CreateDirectoryTree(*SideloadPath))
			{
				return FailAndQueueCleanup(
					FString::Printf(TEXT("Failed to create sideload root directory: %s"), *SideloadPath));
			}

			if (PlatformFile.DirectoryExists(*WorkingModPath) &&
				!PlatformFile.DeleteDirectoryRecursively(*WorkingModPath))
			{
				return FailAndQueueCleanup(
					FString::Printf(TEXT("Failed to clear working sideload directory: %s"), *WorkingModPath));
			}

			// Best-effort cleanup for stale automation fixtures from previous failed runs.
			{
				const FString AutomationFixturePrefix = TEXT("__AutomationSideLoadFixture_");
				PlatformFile.IterateDirectory(
					*SideloadPath,
					[&PlatformFile, &AutomationFixturePrefix](const TCHAR* Path, bool bIsDirectory) {
						if (bIsDirectory)
						{
							const FString FolderName = FPaths::GetCleanFilename(Path);
							if (FolderName.StartsWith(AutomationFixturePrefix, ESearchCase::CaseSensitive))
							{
								PlatformFile.DeleteDirectoryRecursively(Path);
							}
						}
						return true;
					});
			}

			AdvanceState(ETestState::CopyMod);
			return false; // loop again next frame
		}

		case ETestState::CopyMod:
		{
			// Baseline query map count
			GetMapsFromAssetManager(MapCountBeforeMod);
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Baseline map count: %d"), MapCountBeforeMod);
			UGCSubsystem->EnumerateAllUGCPackages([this](const FUGCPackage& Package) {
				BaselinePackageCount++;
				return true;
			});
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Baseline UGC package count: %d"), BaselinePackageCount);

			// Check for available test mods inside TestFixtures
			if (!PlatformFile.DirectoryExists(*TestFixturePath))
			{
				return FailAndQueueCleanup(FString::Printf(
					TEXT("Test fixture directory missing: %s. You MUST place a compiled UGC mod inside this folder."),
					*TestFixturePath));
			}

			// Deterministic fixture selection: sort folder names and pick the first one.
			TArray<FString> FixtureFolders;
			PlatformFile.IterateDirectory(*TestFixturePath, [&FixtureFolders](const TCHAR* FilenameOrDirectory,
																			 bool bIsDirectory) {
				if (bIsDirectory)
				{
					FixtureFolders.Add(FilenameOrDirectory);
				}
				return true;
			});
			FixtureFolders.Sort();

			if (FixtureFolders.Num() == 0)
			{
				return FailAndQueueCleanup(FString::Printf(
					TEXT("Test fixture directory %s is empty. Please place a compiled UGC mod directory inside it."),
					*TestFixturePath));
			}

			const FString SourceModDir = FixtureFolders[0];
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Selected fixture folder: %s"), *SourceModDir);

			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Copying test mod from %s to %s"), *SourceModDir,
				   *WorkingModPath);
			if (!PlatformFile.CopyDirectoryTree(*WorkingModPath, *SourceModDir, true))
			{
				return FailAndQueueCleanup(TEXT("Failed to copy test mod fixture."));
			}

			TArray<FString> PluginDescriptorFiles;
			FileManager.FindFilesRecursive(PluginDescriptorFiles, *WorkingModPath, TEXT("*.uplugin"), true, false);
			PluginDescriptorFiles.Sort();
			if (PluginDescriptorFiles.Num() == 0)
			{
				return FailAndQueueCleanup(TEXT("Test fixture is missing a .uplugin descriptor."));
			}

			const FString SelectedDescriptorPath = PluginDescriptorFiles[0];
			ExpectedPluginName = FPaths::GetBaseFilename(SelectedDescriptorPath);
			ExpectedAssetPath = FString::Printf(TEXT("/%s"), *ExpectedPluginName);
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Expected plugin: %s (%s)"), *ExpectedPluginName,
				   *SelectedDescriptorPath);

			ConflictingProjectPluginPath = FPaths::ProjectDir() / TEXT("Mods") / ExpectedPluginName /
				FString::Printf(TEXT("%s.uplugin"), *ExpectedPluginName);
			ConflictingProjectPluginDisabledPath = ConflictingProjectPluginPath + TEXT(".disabled_by_automation_test");

			if (PlatformFile.FileExists(*ConflictingProjectPluginPath))
			{
				UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Temporarily disabling conflicting project plugin: %s"),
					   *ConflictingProjectPluginPath);
				if (!PlatformFile.MoveFile(*ConflictingProjectPluginDisabledPath, *ConflictingProjectPluginPath))
				{
					return FailAndQueueCleanup(FString::Printf(
						TEXT("Failed to disable conflicting project plugin file: %s"),
						*ConflictingProjectPluginPath));
				}
			}

			AdvanceState(ETestState::InitializeProvider);
			return false;
		}

		case ETestState::InitializeProvider:
		{
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Initializing SideLoad provider"));

			// Setup subsystem with sideload provider
			GetSubsystemProvider(UGCSubsystem, OriginalProvider);
			if (OriginalProvider.GetObject())
			{
				OriginalProviderObject = OriginalProvider.GetObject();
				OriginalProviderObject->AddToRoot();
			}

			USideLoadUGCProvider* SideLoadProvider = NewObject<USideLoadUGCProvider>();
			UGCSubsystem->SetUGCProvider(SideLoadProvider);

			// Ensure delegates are bound
			FOnUGCProviderInitializedDelegate InitDelegate;
			InitDelegate.BindDynamic(HelperObject, &USideLoadTestHelper::OnProviderInitialized);
			UGCSubsystem->InitializeUGCProvider(InitDelegate);

			AdvanceState(ETestState::VerifyLoaded);
			return false;
		}

		case ETestState::VerifyLoaded:
		{
			// Wait for initialization callback with timeout
			if (!bLoaded)
			{
				if (bProviderInitFailed)
				{
					return FailAndQueueCleanup(TEXT("SideLoad provider initialization callback reported failure."));
				}
				if (HasStateTimedOut(30.0))
				{
					return FailAndQueueCleanup(TEXT("Timed out waiting for SideLoad provider initialization callback."));
				}
				return false;
			}

			// 1. Verify expected UGC package was discovered by the subsystem
			int32 FoundPackagesCount = 0;
			bool bExpectedPackageFound = false;
			UGCSubsystem->EnumerateAllUGCPackages([&FoundPackagesCount](const FUGCPackage& Package) {
				FoundPackagesCount++;
				return true;
			});
			UGCSubsystem->EnumerateAllUGCPackages([&bExpectedPackageFound, this](const FUGCPackage& Package) {
				const FString PackagePluginName = FPaths::GetBaseFilename(Package.DescriptorPath);
				if (PackagePluginName.Equals(ExpectedPluginName, ESearchCase::IgnoreCase))
				{
					bExpectedPackageFound = true;
				}
				return true;
			});

			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Found %d UGC packages"), FoundPackagesCount);
			TestTrue(TEXT("UGC package count after load should be > 0"), FoundPackagesCount > 0);
			TestTrue(TEXT("Expected sideloaded package was discovered"), bExpectedPackageFound);

			if (!bExpectedPackageFound)
			{
				return FailAndQueueCleanup(TEXT("Expected sideloaded package was not found after provider refresh."));
			}

			// 2. Check AssetRegistry directly for mod content (works even without UUGC_Metadata)
			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
			TArray<FAssetData> ModAssets;
			AssetRegistry.GetAssetsByPath(FName(*ExpectedAssetPath), ModAssets, true);

			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("AssetRegistry found %d assets under %s"), ModAssets.Num(),
				   *ExpectedAssetPath);
			TestTrue(TEXT("AssetRegistry contains mod assets after load"), ModAssets.Num() > 0);

			// Log individual assets found
			for (const FAssetData& Asset : ModAssets)
			{
				UE_LOG(LogActionRpgAutomationTest, Log, TEXT("  Mod asset: %s (%s)"),
					   *Asset.AssetName.ToString(), *Asset.AssetClassPath.ToString());
			}

			// 3. Also check AssetManager primary assets (may fail due to metadata not loading in editor)
			int32 MapCountAfterMod = 0;
			GetMapsFromAssetManager(MapCountAfterMod);

			UE_LOG(LogActionRpgAutomationTest, Log,
				   TEXT("AssetManager primary map count after mod loaded: %d (before: %d)"),
				   MapCountAfterMod, MapCountBeforeMod);
			if (MapCountAfterMod <= MapCountBeforeMod)
			{
				AddWarning(TEXT("No new maps discovered via AssetManager.GetPrimaryAssetIdList. This is expected when "
								TEXT("UUGC_Metadata cannot be loaded in editor/IoStore mode.")));
			}

			AdvanceState(ETestState::LoadMap);
			return false;
		}

		case ETestState::LoadMap:
		{
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Attempting to load discovered map into Editor..."));

			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
			TArray<FAssetData> ModAssets;
			AssetRegistry.GetAssetsByPath(FName(*ExpectedAssetPath), ModAssets, true);

			FString MapPackageNameToLoad;
			const FName WorldAssetName = UWorld::StaticClass()->GetClassPathName().GetAssetName();
			for (const FAssetData& Asset : ModAssets)
			{
				if (Asset.AssetClassPath.GetAssetName() == WorldAssetName ||
					Asset.AssetClassPath.GetAssetName() == FName("World"))
				{
					MapPackageNameToLoad = Asset.PackageName.ToString();
					break;
				}
			}

			if (MapPackageNameToLoad.IsEmpty())
			{
				return FailAndQueueCleanup(TEXT("Could not find any World/Map assets in the loaded mod package."));
			}

			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Calling AutomationOpenMap for: %s"),
				   *MapPackageNameToLoad);

			// Actually load the map and start PIE (Play In Editor) so it acts like a player
			bool bLoadSuccess = AutomationOpenMap(MapPackageNameToLoad);
			TestTrue(TEXT("AutomationOpenMap returned true"), bLoadSuccess);

			AdvanceState(ETestState::WaitForMapLoad);
			StateStartSeconds = FPlatformTime::Seconds();
			return false;
		}

		case ETestState::WaitForMapLoad:
		{
			// Wait for the PIE world to finish loading and transition into actual gameplay
			UWorld* TestWorld = AutomationCommon::GetAnyGameWorld();
			if (TestWorld && TestWorld->AreActorsInitialized())
			{
				AGameStateBase* GameState = TestWorld->GetGameState();
				if (GameState && GameState->HasMatchStarted())
				{
					UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Map fully loaded and started match in PIE!"));
					AdvanceState(ETestState::CloseMap);
					return false;
				}
			}

			if (FPlatformTime::Seconds() - StateStartSeconds > 15.0)
			{
				return FailAndQueueCleanup(TEXT("Timeout waiting for map to load in PIE."));
			}

			return false; // keep ticking
		}

		case ETestState::CloseMap:
		{
			// Load the default project map to release all references to the UGC map.
			// The editor holds the loaded world/level objects in memory, preventing
			// clean UGC unmounting unless we navigate away first.
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Closing UGC map by loading default project map..."));
			AutomationOpenMap(TEXT("/Game/Maps/ActionRPG_Main"));

			// Force garbage collection to release stale world/level references
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

			UE_LOG(LogActionRpgAutomationTest, Log,
				   TEXT("Default map loaded, GC complete. Proceeding to unload UGC."));
			AdvanceState(ETestState::UnloadMod);
			return false;
		}

		case ETestState::UnloadMod:
		{
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Unloading UGC..."));

			UGCSubsystem->UnloadAllUGCPackages();
			bUnloaded = true;

			AdvanceState(ETestState::VerifyUnloaded);
			return false;
		}

		case ETestState::VerifyUnloaded:
		{
			// wait for callback
			if (!bUnloaded)
			{
				return false;
			}

			// Verify UGC packages are gone
			int32 PackagesAfterUnload = 0;
			bool bExpectedPackageStillLoaded = false;
			UGCSubsystem->EnumerateAllUGCPackages(
				[&PackagesAfterUnload, &bExpectedPackageStillLoaded, this](const FUGCPackage& Package) {
					PackagesAfterUnload++;
					const FString PackagePluginName = FPaths::GetBaseFilename(Package.DescriptorPath);
					if (PackagePluginName.Equals(ExpectedPluginName, ESearchCase::IgnoreCase))
					{
						bExpectedPackageStillLoaded = true;
					}
					return true;
				});
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("UGC packages after unload: %d"), PackagesAfterUnload);
			TestFalse(TEXT("Expected sideloaded package should be unloaded"), bExpectedPackageStillLoaded);

			// Verify AssetRegistry no longer has mod assets
			IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
			TArray<FAssetData> ModAssetsAfterUnload;
			AssetRegistry.GetAssetsByPath(FName(*ExpectedAssetPath), ModAssetsAfterUnload, true);
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("AssetRegistry mod assets after unload under %s: %d"),
				   *ExpectedAssetPath, ModAssetsAfterUnload.Num());
			TestEqual(TEXT("AssetRegistry mod assets should be 0 after unload"), ModAssetsAfterUnload.Num(), 0);

			// Check AssetManager map count
			int32 MapCountAfterUnload = 0;
			GetMapsFromAssetManager(MapCountAfterUnload);
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("AssetManager map count after unload: %d"),
				   MapCountAfterUnload);

			AdvanceState(ETestState::FinalCleanup);
			return false;
		}

		case ETestState::FinalCleanup:
		{
			UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Final cleanup started"));

			if (!ConflictingProjectPluginDisabledPath.IsEmpty() &&
				PlatformFile.FileExists(*ConflictingProjectPluginDisabledPath))
			{
				UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Restoring temporarily disabled project plugin: %s"),
					   *ConflictingProjectPluginPath);
				if (!PlatformFile.MoveFile(*ConflictingProjectPluginPath, *ConflictingProjectPluginDisabledPath))
				{
					AddWarning(FString::Printf(TEXT("Failed to restore project plugin descriptor: %s"),
											   *ConflictingProjectPluginPath));
				}
			}

			if (PlatformFile.DirectoryExists(*WorkingModPath))
			{
				if (UGCSubsystem)
				{
					UGCSubsystem->UnloadAllUGCPackages();
				}

				UE_LOG(LogActionRpgAutomationTest, Log, TEXT("Removing working sideload directory: %s"),
					   *WorkingModPath);

				bool bDeletedWorkingDir = false;
				constexpr int32 DeleteRetryCount = 20;
				constexpr float DeleteRetryDelaySeconds = 0.1f;
				for (int32 Attempt = 0; Attempt < DeleteRetryCount; ++Attempt)
				{
					if (!PlatformFile.DirectoryExists(*WorkingModPath))
					{
						bDeletedWorkingDir = true;
						break;
					}

					if (PlatformFile.DeleteDirectoryRecursively(*WorkingModPath))
					{
						bDeletedWorkingDir = true;
						break;
					}

					FPlatformProcess::Sleep(DeleteRetryDelaySeconds);
				}

				if (!bDeletedWorkingDir)
				{
					UE_LOG(LogActionRpgAutomationTest, Log,
						   TEXT("Working sideload directory could not be deleted yet (likely file lock); it will be "
								"pruned on next test run: %s"),
						   *WorkingModPath);
				}
			}

			// Restore original provider to avoid cross-test contamination.
			if (UGCSubsystem && OriginalProvider.GetObject())
			{
				UGCSubsystem->SetUGCProvider(OriginalProvider);
			}
			else if (UGCSubsystem)
			{
				UGCSubsystem->SetUGCProvider(TScriptInterface<IUGCProvider>());
			}

			if (OriginalProviderObject)
			{
				OriginalProviderObject->RemoveFromRoot();
				OriginalProviderObject = nullptr;
			}

			if (HelperObject)
			{
				HelperObject->TestInstance = nullptr;
				HelperObject->RemoveFromRoot();
				HelperObject = nullptr;
			}

			AdvanceState(ETestState::Done);
			return false;
		}

		case ETestState::Done:
		default:
			return true; // Done
	}
}

IMPLEMENT_CUSTOM_COMPLEX_AUTOMATION_TEST(FSideLoadModTest_FullLifecycle, FSideLoadModTestBase,
										 "ActionRPG.UGC.SideLoadProvider.FullLifecycle",
										 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FSideLoadModTest_FullLifecycle::GetTests(TArray<FString>& OutBeautifiedNames,
											  TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(FString());
}

bool FSideLoadModTest_FullLifecycle::RunTest(const FString& Parameters)
{
	return FSideLoadModTestBase::RunTest(Parameters);
}

#endif // WITH_EDITOR
