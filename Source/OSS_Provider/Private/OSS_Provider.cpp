#include "OSS_Provider.h"

#define LOCTEXT_NAMESPACE "FOSS_ProviderModule"

void FOSS_ProviderModule::StartupModule()
{
	FString AuthProvider;
	if (!FParse::Value(FCommandLine::Get(), TEXT("auth="), AuthProvider))
	{
		GConfig->GetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), AuthProvider, GEngineIni);
	}
	
	
	GConfig->SetString(TEXT("OnlineSubsystem"), TEXT("DefaultPlatformService"), *AuthProvider, GEngineIni);
	GConfig->SetString(TEXT("OnlineSubsystem"), TEXT("NativePlatformService"), *AuthProvider, GEngineIni);
}

void FOSS_ProviderModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FOSS_ProviderModule, OSS_Provider)