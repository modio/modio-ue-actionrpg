// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGGameModeBase.h"

#include "GameFramework/PlayerStart.h"
#include "RPGGameStateBase.h"
#include "RPGPlayerControllerBase.h"

ARPGGameModeBase::ARPGGameModeBase()
{
	GameStateClass = ARPGGameStateBase::StaticClass();
	PlayerControllerClass = ARPGPlayerControllerBase::StaticClass();
	bGameOver = false;
}

void ARPGGameModeBase::ResetLevel()
{
	K2_DoRestart();
}

bool ARPGGameModeBase::HasMatchEnded() const
{
	return bGameOver;
}

void ARPGGameModeBase::GameOver()
{
	if (bGameOver == false)
	{
		K2_OnGameOver();
		bGameOver = true;
	}
}

AActor* ARPGGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> PlayerStarts;
	AActor* ChosenStart;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);
	ChosenStart = PlayerStarts[CurrentPlayerStartIndex];

	CurrentPlayerStartIndex = uint8((CurrentPlayerStartIndex + 1) % PlayerStarts.Num());

	return ChosenStart;
}
