#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	int idx = 0;
	for (auto line : m_pNavMeshPolygon->GetLines())
	{		
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index).size() > 0)
		{
			Vector2 middleOfLine = (line->p2 - line->p1) / 2 + line->p1;
			NavGraphNode* pNavGraphNode = new NavGraphNode(idx, line->index, middleOfLine);
			Graph2D::AddNode(pNavGraphNode);
			idx++;
		}
	}

	std::vector<NavGraphNode*> nodesInTriangles;
	//2. Create connections now that every node is created
	for (auto triangle : m_pNavMeshPolygon->GetTriangles())
	{
		for (auto lineIdx : triangle->metaData.IndexLines)
		{
			for (auto node : Graph2D::GetAllActiveNodes())
			{
				if (node->GetLineIndex() == lineIdx)
				{
					nodesInTriangles.push_back(node);
				}
			}
		}

		if (nodesInTriangles.size() == 2)
		{
			GraphConnection2D* connection = new GraphConnection2D(nodesInTriangles[0]->GetIndex(), nodesInTriangles[1]->GetIndex());
			Graph2D::AddConnection(connection);
		}

		if (nodesInTriangles.size() == 3)
		{
			GraphConnection2D* connection1 = new GraphConnection2D(nodesInTriangles[0]->GetIndex(), nodesInTriangles[1]->GetIndex());
			GraphConnection2D* connection2 = new GraphConnection2D(nodesInTriangles[1]->GetIndex(), nodesInTriangles[2]->GetIndex());
			GraphConnection2D* connection3 = new GraphConnection2D(nodesInTriangles[0]->GetIndex(), nodesInTriangles[2]->GetIndex());

			Graph2D::AddConnection(connection1);
			Graph2D::AddConnection(connection2);
			Graph2D::AddConnection(connection3);
		}

		nodesInTriangles.clear();
	}

	//3. Set the connections cost to the actual distance
	Graph2D::SetConnectionCostsToDistance();
}

