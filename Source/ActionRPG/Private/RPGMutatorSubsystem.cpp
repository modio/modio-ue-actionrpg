/*
 *  Copyright (C) 2026 mod.io Pty Ltd. <https://mod.io>
 *
 *  Distributed under the MIT License. (See accompanying file LICENSE or
 *   view online at <https://github.com/modio/modio-ue-modiougc/blob/main/LICENSE>)
 *
 */


#include "RPGMutatorSubsystem.h"
#include "RPGMutator.h"

MUTATOR_EVENTS_START
IMPLEMENT_MUTATOR_EVENT(EnemyWaveEnded)
IMPLEMENT_MUTATOR_EVENT_RETURN(ModifyDamage)
IMPLEMENT_MUTATOR_EVENT_RETURN(ScorePoints)
MUTATOR_EVENTS_END