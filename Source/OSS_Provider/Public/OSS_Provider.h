#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FOSS_ProviderModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
