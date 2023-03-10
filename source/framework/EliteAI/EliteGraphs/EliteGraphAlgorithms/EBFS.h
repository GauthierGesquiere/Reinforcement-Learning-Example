#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode)
	{
		std::queue<T_NodeType*> openlist; //Nodes that still need a check
		std::map<T_NodeType*, T_NodeType*> closedList; // already checked nodes

		openlist.push(pStartNode);

		while (!openlist.empty())
		{
			T_NodeType* pCurrentNode = openlist.front();	//taking a node from the openlist
			openlist.pop(); //removing that node

			if (pCurrentNode == pDestinationNode)
			{
				break;
			}
			for (auto con : m_pGraph->GetNodeConnections(pCurrentNode))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(con->GetTo());
				if (closedList.find(pNextNode) == closedList.end())
				{
					//we did not find this node in de closedlist
					openlist.push(pNextNode);
					closedList[pNextNode] = pCurrentNode;
				}
			}
		}

		//Start tracking the path
		vector<T_NodeType*> path;
		T_NodeType* pCurrentNode = pDestinationNode;

		while (pCurrentNode != pStartNode)
		{
			path.push_back(pCurrentNode);
			pCurrentNode = closedList[pCurrentNode];
		}
		path.push_back(pStartNode);
		std::reverse(path.begin(), path.end()); //reversing path so the beginning is at the green dot

		return path;
	}
}

