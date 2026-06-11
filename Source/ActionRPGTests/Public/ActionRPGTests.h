#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "UObject/Object.h"
#include "ActionRPGTests.generated.h"

// Forward declaration
class FSideLoadModTestBase;

UCLASS()
class USideLoadTestHelper : public UObject
{
	GENERATED_BODY()

public:
	FSideLoadModTestBase* TestInstance = nullptr;

	UFUNCTION()
	void OnProviderInitialized(bool bSuccess);
};

class FActionRPGTestsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
