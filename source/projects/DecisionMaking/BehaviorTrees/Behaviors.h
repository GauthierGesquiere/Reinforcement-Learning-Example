/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------



//ACTIONS
//******

//WANDER
Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackBoard)
{
	AgarioAgent* pAgent = nullptr;
	pBlackBoard->GetData("Agent", pAgent);

	if (!pAgent)
	{
		return Elite::BehaviorState::Failure;
	}

	pAgent->SetToWander();
	return Elite::BehaviorState::Success;
}


//SEEK
Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackBoard)
{
	AgarioAgent* pAgent = nullptr;
	pBlackBoard->GetData("Agent", pAgent);

	Elite::Vector2 target = Elite::Vector2();
	pBlackBoard->GetData("Target", target);

	if (!pAgent)
	{
		return Elite::BehaviorState::Failure;
	}

	pAgent->SetToSeek(target);
	return Elite::BehaviorState::Success;
}


//CONDITIONS
//**********

bool IsFoodNearby(Elite::Blackboard* pBlackBoard)
{
	AgarioAgent* pAgent = nullptr;
	pBlackBoard->GetData("Agent", pAgent);

	std::vector<AgarioFood*>* foods;
	pBlackBoard->GetData("FoodVec", foods);

	//Check if there is food
	if (foods->size() < 1)
	{
		return false;
	}

	//Get the closest food
	AgarioFood* closestFood = (*foods)[0];
	float closestDistance = Elite::DistanceSquared(pAgent->GetPosition(), closestFood->GetPosition());

	//Calculate the distance squared between agent and the current food
	for (size_t i = 0; i < foods->size(); i++)
	{
		if (Elite::DistanceSquared(pAgent->GetPosition(), (*foods)[i]->GetPosition()) < closestDistance)
		{
			closestFood = (*foods)[i];
			closestDistance = Elite::DistanceSquared(pAgent->GetPosition(), (*foods)[i]->GetPosition());
		}
	}

	//Check if closest food is in a certain range
	const float range = 20.0f;
	if (closestDistance < range * range)
	{
		pBlackBoard->ChangeData("Target", closestFood->GetPosition());
		return true;
	}
	return false;
}

#endif