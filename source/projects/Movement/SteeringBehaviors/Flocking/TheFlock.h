#pragma once
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"

class CellSpace;
class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;


#define USE_SPACE_PARTITIONING

class Flock
{
public:
	Flock(
		int flockSize = 50,
		float worldSize = 100.f,
		SteeringAgent* pAgentToEvade = nullptr,
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);
	vector<SteeringAgent*>& GetNeighbors()  { return m_Neighbors; }

#ifdef USE_SPACE_PARTITIONING
	int& GetNrOfNeighbors() { return m_NrOfNeighbors; }
	bool EnableDebugRendering = false;

#else // No space partitioning
	void RegisterNeighbors(SteeringAgent* pAgent);
#endif // USE_SPACE_PARTITIONING

	void SetSeekTarget(TargetData target);
	void SetWorldTrimSize(float size) { m_WorldSize = size; }
	float GetWorldTrimSize() { return m_WorldSize; }
	const int getNrOfAgents() const { return m_Agents.size(); }

private:
	//Datamembers
	int m_FlockSize = 0;
	vector<SteeringAgent*> m_Agents;
	vector<SteeringAgent*> m_Neighbors; //moet niet deleten want ze pointen naar agent en die worden al verwijderd

	bool m_TrimWorld /*= false*/;
	float m_WorldSize /*= 0.f*/;

	float m_NeighborhoodRadius /*= 10.f*/;
	int m_NrOfNeighbors /*= 0*/;
	
#ifdef USE_SPACE_PARTITIONING

	CellSpace* m_pCellSpace = nullptr;
	vector<Elite::Vector2> m_OldPositions = {};

#endif // USE_SPACE_PARTITIONING

	SteeringAgent* m_pAgentToEvade = nullptr;
	
	//Steering Behaviors
	Separation* m_pSeparationBehavior = nullptr;
	Cohesion* m_pCohesionBehavior = nullptr;
	Alignment* m_pAlignmenthBehavior = nullptr;
	Seek* m_pSeekBehavior = nullptr;
	Wander* m_pWanderBehavior = nullptr;
	Evade* m_pEvadeBehavior = nullptr;

	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;

	float* GetWeight(ISteeringBehavior* pBehaviour);
	Elite::Vector2 CalcVector(Elite::Vector2 vector1, Elite::Vector2 vector2) { return vector1 - vector2; }

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};