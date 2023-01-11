#include "stdafx.h"
#include "Cell.h"

Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox = { {left, bottom}, width, height };
}

Cell::~Cell()
{
}

std::vector<Elite::Vector2> Cell::getRectPoints() const
{
	std::vector<Elite::Vector2> vector;

	vector.push_back({ boundingBox.bottomLeft });
	vector.push_back({ boundingBox.bottomLeft.x + boundingBox.width, boundingBox.bottomLeft.y });
	vector.push_back({ boundingBox.bottomLeft.x + boundingBox.width, boundingBox.bottomLeft.y + boundingBox.height });
	vector.push_back({ boundingBox.bottomLeft.x, boundingBox.bottomLeft.y + boundingBox.height });
	//vector.push_back({ boundingBox.bottomLeft });

	return vector;
}

const char* Cell::GetNrOfAgents() const
{
	if (agents.size() < 1)
	{
		return "0";
	}

	if (agents.size() < 2)
	{
		return "1";
	}

	if (agents.size() < 3)
	{
		return "2";
	}

	if (agents.size() < 4)
	{
		return "3";
	}

	if (agents.size() < 5)
	{
		return "4";
	}

	if (agents.size() < 6)
	{
		return "5";
	}
	
	if (agents.size() < 7)
	{
		return "6";
	}

	if (agents.size() < 8)
	{
		return "7";
	}

	if (agents.size() < 9)
	{
		return "8";
	}

	if (agents.size() < 10)
	{
		return "9";
	}
	return "0";
}
