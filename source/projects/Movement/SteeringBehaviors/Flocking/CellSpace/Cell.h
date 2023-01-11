#pragma once
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

struct Cell
{
	Cell(float left, float bottom, float width, float height);
	~Cell();

	std::vector<Elite::Vector2> getRectPoints() const;
	const char* GetNrOfAgents() const;

	//all the agents currently in this cell
	std::list<SteeringAgent*> agents;
	Elite::Rect boundingBox;
};

