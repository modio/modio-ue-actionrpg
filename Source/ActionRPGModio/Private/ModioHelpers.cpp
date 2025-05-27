/*
 *  Copyright (C) 2025 mod.io Pty Ltd. <https://mod.io>
 *
 *  This file is part of the mod.io Action RPG demo project.
 *
 */

#include "ModioHelpers.h"

#include "ModioSettings.h"

bool UModioHelpers::StringToModioGameEnvironment(const FString& Environment, EModioEnvironment& OutEnvironment)
{
	if (Environment.Equals("Test", ESearchCase::IgnoreCase))
	{
		OutEnvironment = EModioEnvironment::Test;
		return true;
	}
	if (Environment.Equals("Live", ESearchCase::IgnoreCase))
	{
		OutEnvironment = EModioEnvironment::Live;
		return true;
	}
	return false;
}

bool UModioHelpers::StringToModioPortal(const FString& Portal, EModioPortal& OutPortal)
{
	if (Portal.Equals("Apple", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::Apple;
		return true;
	}
	if (Portal.Equals("EpicGamesStore", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::EpicGamesStore;
		return true;
	}
	if (Portal.Equals("GOG", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::GOG;
		return true;
	}
	if (Portal.Equals("Google", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::Google;
		return true;
	}
	if (Portal.Equals("Itchio", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::Itchio;
		return true;
	}
	if (Portal.Equals("Nintendo", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::Nintendo;
		return true;
	}
	if (Portal.Equals("PSN", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::PSN;
		return true;
	}
	if (Portal.Equals("Steam", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::Steam;
		return true;
	}
	if (Portal.Equals("XboxLive", ESearchCase::IgnoreCase))
	{
		OutPortal = EModioPortal::XboxLive;
		return true;
	}
	return false;
}