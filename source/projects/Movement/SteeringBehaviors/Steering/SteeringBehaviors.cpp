//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework/EliteMath/EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	//... logic
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	//std::cout << pAgent->GetRotation() << std::endl;
	return steering.LinearVelocity;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	float distanceToTarget = Distance(pAgent->GetPosition(), m_Target.Position);
	if (distanceToTarget > m_FleeRadius)
	{
		return SteeringOutput(Elite::ZeroVector2, 0.f, false);
	}

	//... logic
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), -steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	return -steering.LinearVelocity;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	//... logic
	//Variables
	const float maxSpeed = 50.0f;
	const float slowRadius = 15.f;

	float distance;

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	distance = steering.LinearVelocity.Magnitude();
	steering.LinearVelocity.Normalize();

	if (distance < slowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distance / slowRadius;
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), Elite::Color{ 0, 1, 0, 1 });
	}
	return steering.LinearVelocity;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	//... logic	
	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	Elite::Vector2 dirAgent = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity = Elite::Vector2{ 0.0f , 0.0f };


	//std::cout << pAgent->GetRotation() << std::endl;
	pAgent->SetAutoOrient(false);
	if (dirAgent.x <= 0)
	{
		pAgent->SetRotation((float)atan((double)dirAgent.y / (double)dirAgent.x) - 2 * M_PI / 4);
	}
	else
	{
		pAgent->SetRotation((float)atan((double)dirAgent.y / (double)dirAgent.x) + 2 * M_PI / 4);
	}
	return steering.LinearVelocity;
}

//Wander
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 agentDir = pAgent->GetDirection();
	agentDir.Normalize();

	float angle = atan(agentDir.y / agentDir.x);
	float angleInDegrees = Elite::ToDegrees(angle);

	float randAngleDegrees = float(rand() % 90 - 45);
	float randAngleRad = Elite::ToRadians(randAngleDegrees);


	if (agentDir.x == 0 && agentDir.y == 0)
	{
		angle = float(rand() % 90 - 45);
	}


	if (angle < 0)
	{
		angle *= -1;
	}


	float newAngle = randAngleRad + angle;
	float newAngleInDegrees = Elite::ToDegrees(newAngle);

	if (newAngle < 0)
	{
		newAngle *= -1;
	}

	float Xvalue = cos(newAngle);
	float YValue = sin(newAngle);

	if (agentDir.x > 0)
	{
		if (agentDir.y < 0)
		{
			YValue *= -1;
		}
	}
	if (agentDir.x < 0)
	{
		Xvalue *= -1;

		if (agentDir.y < 0)
		{
			YValue *= -1;
		}
	}

	Elite::Vector2 newDir{ Xvalue, YValue };

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetDirection(), 5.f, Elite::Color{ 0, 1, 0, 1 });
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), newDir, 5.0f, Elite::Color{ 0, 0, 1, 1 });
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition() + (pAgent->GetDirection().GetNormalized() * 5.f), 4.f, Elite::Color{ 1, 0, 0, 1 }, 0.0f);
		//DEBUGRENDERER2D->DrawCircle(p2, 2.f, Elite::Color{ 0, 0, 1, 1 }, 0.0f);
	}

	Elite::Vector2 agentVelocity = pAgent->GetDirection();

	steering.LinearVelocity = newDir;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//... logic
	return steering.LinearVelocity;
}

//Pursuit
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	//... logic
	Elite::Vector2 forwardVelocity{ pAgent->GetDirection() };
	forwardVelocity.Normalize();
	//steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	Elite::Vector2 desiredVelocity{ m_Target.Position - pAgent->GetPosition() + 3 * m_Target.LinearVelocity };
	desiredVelocity.Normalize();
	Elite::Vector2 steeringVelocity{ forwardVelocity + desiredVelocity };
	steeringVelocity.Normalize();
	steeringVelocity *= 5;

	steering.LinearVelocity = steeringVelocity;

	/*if (pAgent->CanRenderBehavior())
	{*/
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), forwardVelocity, 5.f, Elite::Color{ 1, 0, 0, 1 });
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVelocity, 5.f, Elite::Color{ 1, 1, 0, 1 });
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steeringVelocity, steeringVelocity.Magnitude(), Elite::Color{ 0, 0, 1, 1 });
	//}
	//std::cout << pAgent->GetRotation() << std::endl;
	return steering.LinearVelocity;
}

//Evade
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	float distance = Distance(pAgent->GetPosition(), m_Target.Position);
	if (distance > m_Radius)
	{
		return SteeringOutput(Elite::ZeroVector2, 0.f, false);
	}

	//... logic
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition() + 3 * m_Target.LinearVelocity;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), -steering.LinearVelocity, 5.f, Elite::Color{ 0, 1, 0, 1 });
	}
	return -steering.LinearVelocity;
}
