#pragma once

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		//variables
		vector<T_NodeType*> path;
		vector<NodeRecord> openList;
		vector<NodeRecord> closedList;
		NodeRecord currentRecord;

		currentRecord.pNode = pStartNode;
		currentRecord.pConnection = nullptr;

		openList.push_back(currentRecord);
		int indexxxx = currentRecord.pNode->GetIndex();

		while (!openList.empty())
		{
			NodeRecord bestRecord;
			bestRecord.estimatedTotalCost = FLT_MAX;

			for (auto nodeRecord : openList)
			{
				if (bestRecord.estimatedTotalCost > nodeRecord.estimatedTotalCost)
				{
					bestRecord = nodeRecord;
				}
			}

			if (bestRecord.pNode == pGoalNode)
			{				
				break;
			}

			for (auto connections : m_pGraph->GetNodeConnections(bestRecord.pNode))
			{
				NodeRecord newRecord;
				int idx = connections->GetTo();
				newRecord.pNode = m_pGraph->GetNode(idx);
				
				openList.push_back(newRecord);
			}

			vector<NodeRecord> tempList;

			for (size_t i = 0; i < openList.size(); i++)
			{
				if (openList[i] == bestRecord)
				{
					int idx = bestRecord.pNode->GetIndex();
					closedList.push_back(bestRecord);
				}
				else
				{
					tempList.push_back(openList[i]);
				}
			}

			openList.clear();
			openList = tempList;
		}
		
		//Start tracking the path
		NodeRecord CurrentRecordInPath;
		CurrentRecordInPath.pNode = pGoalNode;

		while (CurrentRecordInPath.pNode != pStartNode)
		{
			path.push_back(CurrentRecordInPath.pNode);

			bool breakTrue = false;

			for (auto nodeRecord : closedList)
			{
				for (auto connections : m_pGraph->GetNodeConnections(CurrentRecordInPath.pNode))
				{
					if (nodeRecord.pNode == m_pGraph->GetNode(connections->GetTo()))
					{
						CurrentRecordInPath = nodeRecord;
						breakTrue = true;						
					}

					if (breakTrue)
					{
						break;
					}
				}

				if (breakTrue)
				{
					break;
				}
			}
		}
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end()); //reversing path so the beginning is at the green dot

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}