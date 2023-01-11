#include "stdafx.h"
#include "QZombie.h"
#include "Food.h"

// for setting the precision in cout for floating points.
#include <iomanip>

QZombie::QZombie(float x,
                 float y,
                 float fov,
                 float sFov,
                 float angle,
                 int memorySize,
                 int nrInputs,
                 int nrOutputs,
                 bool useBias,
                 int index)
	: m_Location(x, y), m_StartLocation(x, y), m_FOV(fov), m_SFOV(sFov), m_Angle(angle),
	m_AliveColor(0.9f, 0.5f, .5f),
	m_DeadColor(.75f, 0.1f, .2f),
	m_NrOfInputs(nrInputs),
	m_NrOfOutputs(nrOutputs),
	m_MemorySize(memorySize),
	m_UseBias(useBias),
	m_BotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_DeltaBotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_SAngle(1, nrOutputs),
	m_Index(index)
{
	float start = -m_SFOV / 2;
	float step = m_SFOV / (nrOutputs - 1);
	for (int i = 0; i < nrOutputs; ++i)
	{
		float value = start + i * step;
		m_SAngle.Set(0, i, value);
	}

	m_ActionMatrixMemoryArr = new Elite::FMatrix[m_MemorySize];
	m_StateMatrixMemoryArr = new Elite::FMatrix[m_MemorySize];

	for (int i = 0; i < m_MemorySize; ++i)
	{
		m_StateMatrixMemoryArr[i].Resize(1, m_NrOfInputs + (m_UseBias ? 1 : 0));
		m_ActionMatrixMemoryArr[i].Resize(1, m_NrOfOutputs);
	}
	m_BotBrain.Randomize(-1.0f, 2.0f);
	if (m_UseBias) {
		m_BotBrain.SetRowAll(m_NrOfInputs, -10.0f);
	}

	//m_BotBrain.Print();
}

QZombie::~QZombie()
{
	delete[] m_ActionMatrixMemoryArr;
	delete[] m_StateMatrixMemoryArr;
}

void QZombie::Update(vector<QBot*>& botList, float deltaTime)
{
	if (!m_Alive)
	{
		return;
	}

	m_Age += deltaTime;
	currentIndex = (currentIndex + 1) % m_MemorySize;

	m_Visible.clear();
	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	float angleStep = m_FOV / (m_NrOfInputs);
	m_StateMatrixMemoryArr[currentIndex].SetAll(0.0);
	bool cameClose = false;
	for (QBot* bot : botList) 
	{
		if (!bot->IsAlive()) 
		{
			continue;
		}

		Vector2 botLoc = bot->GetLocation();
		Vector2 botVector = botLoc - (m_Location - dir * 10);
		float dist = (botLoc - m_Location).Magnitude();

		if (dist > m_MaxDistance) 
		{
			continue;
		}

		botVector *= 1 / dist;

		float angle = AngleBetween(dir, botVector);
		if (angle > -m_FOV / 2 && angle < m_FOV / 2) 
		{
			m_Visible.push_back(bot);

			int index = (int)((angle + m_FOV / 2) / angleStep);
			float invDist = CalculateInverseDistance(dist);
			float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, index);
			if (invDist > currentDist) 
			{
				m_StateMatrixMemoryArr[currentIndex].Set(0, index, invDist);
			}
		}
		else if (dist < 10.0f) 
		{
			cameClose = true;
		}
		if (dist < 5.0f) 
		{
			bot->Dead(true);
			m_CameCloseCounter = 50;
			m_BotEaten++;
			m_Health += 30.0f;
			//Reinforcement(m_PositiveQ, m_MemorySize);
		}
	}

	if (m_CameCloseCounter > 0) 
	{
		m_CameCloseCounter--;
	}

	if (cameClose && m_CameCloseCounter == 0) 
	{
		//Reinforcement(m_NegativeQClose, m_MemorySize);
		m_CameCloseCounter = 50;
	}


	m_StateMatrixMemoryArr[currentIndex].Set(0, m_NrOfInputs, 1); //bias
	m_StateMatrixMemoryArr[currentIndex].MatrixMultiply(m_BotBrain, m_ActionMatrixMemoryArr[currentIndex]);
	m_ActionMatrixMemoryArr[currentIndex].Sigmoid();

	int r, c;
	float max = m_ActionMatrixMemoryArr[currentIndex].Max(r, c);

	float dAngle = m_SAngle.Get(0, c);
	m_Angle += dAngle * deltaTime;

	Elite::Vector2 newDir(cos(m_Angle), sin(m_Angle));
	m_Location += newDir * m_Speed * deltaTime;

	m_Health -= 0.1f;
	if (m_Health < 0)
	{
		m_Alive = false;
	}
}

void QZombie::Render(bool vision, bool details)
{
	if (!m_Alive)
	{
		return;
	}

	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	Elite::Vector2 leftVision(cos(m_Angle + m_FOV / 2), sin(m_Angle + m_FOV / 2));
	Elite::Vector2 rightVision(cos(m_Angle - m_FOV / 2), sin(m_Angle - m_FOV / 2));

	Elite::Vector2 perpDir(-dir.y, dir.x);

	Color c = m_DeadColor;
	if (m_Alive) {
		c = m_AliveColor;
	}

	DEBUGRENDERER2D->DrawSolidCircle(m_Location, 2, dir, c);
	if (vision)
	{
		// draw the vision
		if (m_Alive) 
		{
			DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * leftVision, c);
			DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * rightVision, c);

			for (QBot* f : m_Visible) {
				Vector2 loc = f->GetLocation();
				DEBUGRENDERER2D->DrawCircle(loc, 3, c, 0.5f);
			}
		}
	}

	DEBUGRENDERER2D->DrawString(m_Location, to_string((int)m_Health).c_str());

	if (details)
	{
		for (int i = 0; i < m_NrOfInputs; ++i)
		{

			if (m_StateMatrixMemoryArr[currentIndex].Get(0, i) > 0.0f) {
				DEBUGRENDERER2D->DrawSolidCircle(m_Location - 2.5 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_AliveColor);
			}
			else {
				DEBUGRENDERER2D->DrawSolidCircle(m_Location - 3.0 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_DeadColor);
			}
		}

		char age[10];
		snprintf(age, 10, "%.1f seconds", m_Age);
		DEBUGRENDERER2D->DrawString(m_Location + m_MaxDistance * dir, age);
		DEBUGRENDERER2D->DrawString(m_Location - m_MaxDistance / 5 * dir, to_string((int)m_Index).c_str());
	}
}

FMatrix* QZombie::GetBrain()
{
	return &m_BotBrain;
}

void QZombie::SetBrain(FMatrix* matrix)
{
	m_BotBrain.SetAll(0);
	m_BotBrain.Add(*matrix);
}

bool QZombie::IsAlive()
{
	return m_Alive;
}

void QZombie::ResetBot()
{
	// update the bot brain, something went wrong.
	m_Alive = true;
	//Reinforcement(m_NegativeQ, m_MemorySize);
	m_Health = 100.0f;
	m_Location = m_StartLocation;

	m_Age = 0;

	m_TimeOfDeath = 0;
	m_BotEaten = 0;
}

float QZombie::CalculateFitness() const
{
	return m_BotEaten + m_TimeOfDeath;
}

//void QZombie::MutateMatrix(Generation* gen, Elite::FMatrix& matrix, float mutationRate, float mutationAmplitude) 
//{
//	for (int c = 0; c < matrix.GetNrOfColumns(); ++c) 
//	{
//		for (int r = 0; r < matrix.GetNrOfRows(); ++r) 
//		{
//			if (gen->Random(0, 1) < mutationRate) 
//			{
//				float update = gen->Random(-mutationAmplitude, mutationAmplitude);
//				float currentVal = matrix.Get(r, c);
//				matrix.Set(r, c, currentVal + update);
//			}
//		}
//	}
//}

void QZombie::Reinforcement(float factor, int memory)
{
	// go back in time, and reinforce (or inhibit) the weights that led to the right/wrong decision.
	m_DeltaBotBrain.SetAll(0);

	//Get the min of these vars
	int min = m_MemorySize;
	if (m_MemorySize > memory)
	{
		min = memory;
	}

	for (int mi = 0; mi < min; mi++)
	{
		float timeFactor = 1 / (1 + mi * mi);

		int actualIndex = mi - currentIndex;

		if (0 > actualIndex)
		{
			actualIndex = min + actualIndex;
		}

		int r, maxCol;
		m_ActionMatrixMemoryArr[actualIndex].Max(r, maxCol);

		for (int c = 0; c < m_DeltaBotBrain.GetNrOfRows(); c++)
		{
			float value = m_StateMatrixMemoryArr[actualIndex].Get(0, c);
			if (0 < value)
			{
				m_DeltaBotBrain.Add(c, maxCol, timeFactor * factor * value);

				int randC = randomInt(m_DeltaBotBrain.GetNrOfColumns());
				if (randC == maxCol)
				{
					if (randC + 1 == m_DeltaBotBrain.GetNrOfColumns())
					{
						randC = 0;
					}
					else
					{
						randC++;
					}
				}

				m_DeltaBotBrain.Add(c, randC, -timeFactor * factor * value);
			}
		}
	}

	m_DeltaBotBrain.ScalarMultiply(1.0f / static_cast<float>(m_MemorySize));

	m_BotBrain.Add(m_DeltaBotBrain);
}


float QZombie::CalculateInverseDistance(float realDist)
{
	// version 1 
	//return m_MaxDistance - realDist;
	// version 2
	float nDist = realDist / m_MaxDistance;
	float invDistSquared = m_MaxDistance / (1 + nDist * nDist);
	return invDistSquared;
}
