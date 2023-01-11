#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{

	public:
		static std::vector<Elite::Vector2> FindPath(Elite::Vector2 startPos, Elite::Vector2 endPos, Elite::NavGraph* pNavGraph, std::vector<Elite::Vector2>& debugNodePositions, std::vector<Elite::Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Elite::Vector2> finalPath{};

			//Get the start and endTriangle
			Polygon* pNavMesh = pNavGraph->GetNavMeshPolygon();

			//We have valid start/end triangles and they are not the same
			if (pNavMesh->GetTriangleFromPosition(startPos) == nullptr || pNavMesh->GetTriangleFromPosition(endPos) == nullptr)
			{
				return finalPath;
			}
			if (pNavMesh->GetTriangleFromPosition(startPos) == pNavMesh->GetTriangleFromPosition(endPos))
			{
				finalPath.push_back(endPos);
				return finalPath;
			}
			
			//=> Start looking for a path
			//Copy the graph
			NavGraph* pGraphCopy = pNavGraph;

			//Create extra node for the Start Node (Agent's position)
			NavGraphNode* pStartNode = new NavGraphNode(pGraphCopy->GetNrOfActiveNodes(), -1, startPos);

			pGraphCopy->AddNode(pStartNode);
			for (auto lineIdx : pNavMesh->GetTriangleFromPosition(pStartNode->GetPosition())->metaData.IndexLines)
			{
				GraphConnection2D* connection = new GraphConnection2D(pStartNode->GetIndex(), lineIdx, Distance(pGraphCopy->GetNode(lineIdx)->GetPosition(), pStartNode->GetPosition()));
				pGraphCopy->AddConnection(connection);
			}

			//Create extra node for the endNode
			NavGraphNode* pEndNode = new NavGraphNode(pGraphCopy->GetNrOfActiveNodes(), -1, endPos);
			pGraphCopy->AddNode(pEndNode);
			for (auto lineIdx : pNavMesh->GetTriangleFromPosition(pEndNode->GetPosition())->metaData.IndexLines)
			{
				GraphConnection2D* connection = new GraphConnection2D(pEndNode->GetIndex(), lineIdx, Distance(pGraphCopy->GetNode(lineIdx)->GetPosition(), pEndNode->GetPosition()));
				pGraphCopy->AddConnection(connection);
	
			}

			//Run A star on new graph
			Elite::Heuristic m_pHeuristicFunction = Elite::HeuristicFunctions::Chebyshev;
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(pGraphCopy, m_pHeuristicFunction);
			std::vector<NavGraphNode*> pathway = pathfinder.FindPath(pStartNode, pEndNode);

			for (auto node : pathway)
			{
				//finalPath.push_back(node->GetPosition());
				debugNodePositions.push_back(node->GetPosition());
			}

			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(pathway, pGraphCopy->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(debugPortals);

			return finalPath;
		}
	};
}
