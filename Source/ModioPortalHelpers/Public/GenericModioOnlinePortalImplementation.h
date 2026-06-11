/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#pragma once

#include "CoreTypes.h"

#include "OnlineSubsystem.h"

#include "Interfaces/IModioPortalInterface.h"

#include "GenericModioOnlinePortalImplementation.generated.h"

UCLASS()
class MODIOPORTALHELPERS_API UGenericModioOnlinePortalImplementation : public UObject, public IModioPortalInterface
{
	GENERATED_BODY()

};

#if !MODIO_PLATFORM_HELPERS
typedef UGenericModioOnlinePortalImplementation UModioOnlinePortalImplementation;
#endif