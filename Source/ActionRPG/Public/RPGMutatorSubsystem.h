/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-ue-modiougc/blob/main/LICENSE>)
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "Mutators/MutatorSubsystem.h"
#include "Mutators/MutatorUtils.h"
#include "RPGMutatorSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONRPG_API URPGMutatorSubsystem : public UUGCMutatorSubsystem
{
	GENERATED_BODY()

public:

	MUTATOR_EVENTS_START
	/**
	 * Trigger EnemyWaveEnded for registered mutators
	 * @param WaveNumber Which number wave just ended
	 */
	DEFINE_MUTATOR(EnemyWaveEnded, int32, WaveNumber)

	/**
	 * Trigger ModifyDamage for registered mutators
	 * @param Target The Actor about to be damaged
	 * @param Source The Actor responsible for dealing the damage
	 * @param Amount The amount of damage about to be dealt
	 * @return The amount of points after being modified by mutators
	 */
	DEFINE_MUTATOR_RETURN(ModifyDamage, class AActor*, Target, class AActor*, Source, float, Amount)

	/**
	 * Trigger ScorePoints for registered mutators
	 * @param Scorer The controller that is scoring points
	 * @param Amount The amount of points about to be scored
	 * @return The amount of points after being modified by mutators
	 */
	DEFINE_MUTATOR_RETURN(ScorePoints, class AController*, Scorer, int32, Amount)
	MUTATOR_EVENTS_END
	
};
