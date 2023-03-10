/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------

// State that make the Agent Wander
class WanderState : public Elite::FSMState
{
	virtual void OnEnter(Elite::Blackboard* pBlackBoard) override
	{
		AgarioAgent* pAgent = nullptr;
		bool success = pBlackBoard->GetData("Agent", pAgent);

		if (!success)
		{
			return;
		}

		pAgent->SetToWander();
	}
};

//-----------------
//---TRANSITIONS---
//-----------------

#endif