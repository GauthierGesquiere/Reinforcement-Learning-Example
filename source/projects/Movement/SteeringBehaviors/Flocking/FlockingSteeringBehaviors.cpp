#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	vector<SteeringAgent*> neighbors = m_pFlock->GetNeighbors();
	Elite::Vector2 Position;
	Elite::Vector2 AveragePosition;
	int counter = 0;

	for (auto pNeighbor : neighbors)
	{
		Position += pNeighbor->GetPosition();
		counter++;
	}

	AveragePosition = Position / counter;

	//... logic
	steering.LinearVelocity = AveragePosition - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	//std::cout << pAgent->GetRotation() << std::endl;
	return steering.LinearVelocity;
}


//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	vector<SteeringAgent*> neighbors = m_pFlock->GetNeighbors();
	Elite::Vector2 Position;
	Elite::Vector2 AveragePosition;

	for (auto pNeighbor : neighbors)
	{
		Position += (pAgent->GetPosition() - pNeighbor->GetPosition()).GetNormalized() / (pAgent->GetPosition().Distance(pNeighbor->GetPosition()));
	}
	AveragePosition = Position;

	//... logic
	steering.LinearVelocity = AveragePosition;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	//std::cout << pAgent->GetRotation() << std::endl;
	return steering.LinearVelocity;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput Alignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	vector<SteeringAgent*> neighbors = m_pFlock->GetNeighbors();
	Elite::Vector2 Velocity;
	Elite::Vector2 AverageVelocity;
	int counter = 0;

	for (auto pNeighbor : neighbors)
	{
		Velocity += pNeighbor->GetLinearVelocity();
		counter++;
	}
	AverageVelocity = Velocity / counter;

	//... logic
	steering.LinearVelocity = AverageVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	//std::cout << pAgent->GetRotation() << std::endl;
	return steering.LinearVelocity;
}