#include "stdafx.h"
#include "CellSpace.h"

CellSpace::CellSpace(Flock* pFlock)
{
	m_pFlock = pFlock;

	float CellSideWidth = m_pFlock->GetWorldTrimSize() / m_GridCells;
	Elite::Vector2 p1{ 0, m_pFlock->GetWorldTrimSize() }, p2{ 0, m_pFlock->GetWorldTrimSize() - CellSideWidth }, p3{ CellSideWidth, m_pFlock->GetWorldTrimSize() - CellSideWidth }, p4{ CellSideWidth, m_pFlock->GetWorldTrimSize() };

	for (size_t i = 0; i < m_GridCells; i++)
	{
		//in the width
		for (size_t j = 0; j < m_GridCells; j++)
		{
			//in the height
			grid.push_back(new Cell(p2.x, p2.y, CellSideWidth, CellSideWidth));

			p1.y -= CellSideWidth;
			p2.y -= CellSideWidth;
			p3.y -= CellSideWidth;
			p4.y -= CellSideWidth;
		}


		p1.y = m_pFlock->GetWorldTrimSize();
		p2.y = m_pFlock->GetWorldTrimSize() - CellSideWidth;
		p3.y = m_pFlock->GetWorldTrimSize() - CellSideWidth;
		p4.y = m_pFlock->GetWorldTrimSize();

		p1.x += CellSideWidth;
		p2.x += CellSideWidth;
		p3.x += CellSideWidth;
		p4.x += CellSideWidth;
	}

	float currentRadius = 0.0f;
	int AmountOfCellsInRadius = 0;
	while (m_NeighborhoodRadius >= currentRadius)
	{
		currentRadius += CellSideWidth;
		AmountOfCellsInRadius++;
	}
	
}

CellSpace::~CellSpace()
{
	for (auto gridcell : grid)
	{
		SAFE_DELETE(gridcell);
	}
	grid.clear();
}


int CellSpace::PositionToIndex(Elite::Vector2 pos)
{
	int Cellx = pos.x / grid[0]->boundingBox.width;
	int Celly = pos.y / grid[0]->boundingBox.width;

	int Idx = Cellx * m_GridCells + (m_GridCells - 1 - Celly);

	if (Idx >= grid.size() && Idx > 0)
	{
		return Idx - grid.size();
	}

	if (Idx < 0)
	{
		return Idx + grid.size();
	}
	return Idx;

}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	int index = PositionToIndex(agent->GetPosition());
	/*if (index > grid.size() && index > 0)
	{
		index -= grid.size();
	}*/
	grid[index]->agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(float deltaT, SteeringAgent* agent, Elite::Vector2 oldPos)
{
	Elite::Vector2 PosAgent = agent->GetPosition();

	int indexOldPos = PositionToIndex(oldPos);
	int indexPosAgent = PositionToIndex(PosAgent);

	if (indexPosAgent == indexOldPos)
	{
		//still in the same cell
		return;
	}
	AddAgent(agent);
	/*if (indexOldPos > grid.size() && indexOldPos > 0)
	{
		indexOldPos -= grid.size();
	}*/
	if (grid[indexOldPos]->agents.size() != 0)
	{
		grid[indexOldPos]->agents.remove(agent);
	}
}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent)
{
	int agentCell = PositionToIndex(pAgent->GetPosition());
	int CellOnTheRightOfAgent;
	vector<SteeringAgent*> AllAgentsInNeighborhood;
	std::list<int> allCellsInNeighborHood;
	int counter = 0;

	//allCellsInNeighborHood.push_back(agentCell);

	CellOnTheRightOfAgent = PositionToIndex({ pAgent->GetPosition().x + m_NeighborhoodRadius,  pAgent->GetPosition().y });
	allCellsInNeighborHood.push_back(CellOnTheRightOfAgent);

	for (size_t i = agentCell; i < CellOnTheRightOfAgent; i += m_GridCells)
	{
		allCellsInNeighborHood.push_back(i);
		counter++;
	}

	for (size_t i = agentCell; i < CellOnTheRightOfAgent; i += m_GridCells)
	{
		for (size_t j = 1; j <= counter; j++)
		{
			allCellsInNeighborHood.push_back(i + j);
			allCellsInNeighborHood.push_back(i - j);
		}
	}

	int previousCell = agentCell - m_GridCells * counter;

	for (size_t i = 0; i < counter; i++)
	{
		allCellsInNeighborHood.push_back(previousCell);
		for (size_t j = 1; j <= counter; j++)
		{
			allCellsInNeighborHood.push_back(previousCell + j);
			allCellsInNeighborHood.push_back(previousCell - j);
		}
		previousCell += m_GridCells;
	}

	for (size_t i = 1; i <= counter; i++)
	{
		allCellsInNeighborHood.push_back(CellOnTheRightOfAgent + i);
		allCellsInNeighborHood.push_back(CellOnTheRightOfAgent - i);
	}
	std::list<int> copyallCellsInNeighborHood = allCellsInNeighborHood;

	for (auto Cell : allCellsInNeighborHood)
	{
		if (Cell > grid.size() - 1)
		{
			copyallCellsInNeighborHood.remove(Cell);
		}

		if (Cell < 0)
		{
			copyallCellsInNeighborHood.remove(Cell);
		}
	}



	//find neighborCells
	for (auto Cell : copyallCellsInNeighborHood)
	{
		for (auto pOtherAgent : grid[Cell]->agents)
		{
			if (pOtherAgent != pAgent)
			{
				AllAgentsInNeighborhood.push_back(pOtherAgent);
			}
		}
	}

	for (auto pOtherAgent : AllAgentsInNeighborhood)
	{
		Elite::Vector2 VectorBetweenPoints;
		VectorBetweenPoints = CalcVector(pAgent->GetPosition(), pOtherAgent->GetPosition());

		float distanceBetween2Points = sqrt(VectorBetweenPoints.x * VectorBetweenPoints.x + VectorBetweenPoints.y * VectorBetweenPoints.y);

		if (distanceBetween2Points < m_NeighborhoodRadius && distanceBetween2Points != 0.0f)
		{
			m_pFlock->GetNeighbors().push_back(pOtherAgent);

#ifdef USE_SPACE_PARTITIONING

			m_pFlock->GetNrOfNeighbors()++;

#endif
		}
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, Elite::Color{ 1, 0, 0, 1 }, 0.0f);
		
		for (auto Cell : grid)
		{
			Elite::Polygon* polygon{new Elite::Polygon(Cell->getRectPoints()) };
			DEBUGRENDERER2D->DrawPolygon(polygon, {1, 1, 0, 1});

			const char* NrOfAgents = Cell->GetNrOfAgents();

			DEBUGRENDERER2D->DrawString(Cell->getRectPoints()[3], NrOfAgents);
			delete polygon;
		}

	}

}
