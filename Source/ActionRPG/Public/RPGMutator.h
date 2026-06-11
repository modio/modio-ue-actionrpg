/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-ue-modiougc/blob/main/LICENSE>)
 *
 */

#pragma once

#include "CoreMinimal.h"
#include "Mutators/Mutator.h"
#include "RPGMutator.generated.h"

/**
 * 
 */
UCLASS(abstract)
class ACTIONRPG_API URPGMutator : public UUGCMutator
{
	GENERATED_BODY()
	
public:

	MUTATOR_EVENTS_START
	/**
	 * Gets called when an enemy wave has ended
	 * @param WaveIndex The number wave that was just completed
	 */
	DECLARE_MUTATOR_EVENT_BLUEPRINT(EnemyWaveEnded)

	/**
	 * Gets called when damage is about to be dealt
	 * @param Target The actor about to be dealt damage
	 * @param Source The actor about to deal damage
	 * @param Amount The amount of damage to be dealt. May have already been mutated
	 * @return The mutated value of the amount of damage to dealt
	 */
	DECLARE_MUTATOR_EVENT_BLUEPRINT_RETURN(ModifyDamage)

	/**
	 * Gets called when points are about to be scored
	 * @param Scorer The controller about to score points
	 * @param Amount The amount of points to score. May have already been mutated
	 * @return The mutated value of the number of points to score
	 */
	DECLARE_MUTATOR_EVENT_BLUEPRINT_RETURN(ScorePoints)
	MUTATOR_EVENTS_END

};
