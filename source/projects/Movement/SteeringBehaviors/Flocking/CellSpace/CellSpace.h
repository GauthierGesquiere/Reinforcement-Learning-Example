#pragma once
#include "Cell.h"
#include "projects/Movement/SteeringBehaviors/Flocking/TheFlock.h"

class CellSpace
{
public:
	CellSpace(Flock* pFlock);
	~CellSpace();

	Flock* m_pFlock = nullptr;
	int m_NrOfNeighbors = 0;
	vector<SteeringAgent*> m_Neighbors;

	vector<Cell*> grid;
	float m_GridCells = 25.0f;
	float m_NeighborhoodRadius = 5.f;

	Elite::Vector2 CalcVector(Elite::Vector2 vector1, Elite::Vector2 vector2) { return vector1 - vector2; }

	int PositionToIndex(Elite::Vector2 pos);
	void AddAgent(SteeringAgent* agent);

	void UpdateAgentCell(float deltaT, SteeringAgent* agent, Elite::Vector2 oldPos);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const { return m_NrOfNeighbors; }
	const vector<SteeringAgent*>& GetNeighbors() const { return m_Neighbors; }
};

