#include "stdafx.h"
#include "QBot.h"
#include "Food.h"
#include "QZombie.h"

// for setting the precision in cout for floating points.
#include <iomanip>

QBot::QBot(float x,
           float y,
           float fov,
           float sFov,
           float angle,
           int memorySize,
           int nrInputs,
           int nrOutputs,
           bool useBias,
           int index)
:	m_Location(x, y), m_StartLocation(x, y), m_FOV(fov), m_SFOV(sFov), m_Angle(angle),
	m_AliveColor(0.1f, 0.5f, .5f),
	m_DeadColor(.75f, 0.1f, .2f),
	m_NrOfInputs(nrInputs),
	m_NrOfOutputs(nrOutputs),
	m_MemorySize(memorySize),
	m_UseBias(useBias),
	m_BotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_DeltaBotBrain(nrInputs + (useBias ? 1 : 0), nrOutputs),
	m_SAngle(1, 5),
	m_Index(index)
{
	float start = -m_SFOV / 2;
	float step = m_SFOV / (5 - 1);
	for (int i = 0; i < 5; ++i)
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

	if (m_UseBias) 
	{
		m_BotBrain.SetRowAll(m_NrOfInputs, -10.0f);
	}

	//m_BotBrain.Print();
}

QBot::~QBot() 
{
	delete[] m_ActionMatrixMemoryArr;
	delete[] m_StateMatrixMemoryArr;
}

void QBot::Update(vector<Food*>& foodList, vector<QZombie*>& zombieList, float deltaTime)
{
	if (!m_Alive) 
	{
		return;
	}

	m_Age += deltaTime;
	currentIndex = (currentIndex + 1) % m_MemorySize;
	
	m_Visible.clear();
	m_VisibleZombie.clear();

	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	float angleStep = m_FOV / 16;//(m_NrOfInputs);

	//float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, 0);
	//std::cout << currentDist << std::endl;
	m_StateMatrixMemoryArr[currentIndex].SetAll(0.0);
	//m_StateMatrixMemoryArr[currentIndex].Set(0, 0, currentDist);

	bool cameClose = false;
	int checkIfIsTooFar = 0;

	for (Food* food : foodList) 
	{
		Vector2 foodLoc = food->GetLocation();
		Vector2 foodVector = foodLoc - (m_Location - dir * 10);
		float dist = (foodLoc - m_Location).Magnitude();

		if (dist > m_MaxDistance) 
		{
			checkIfIsTooFar++;
			continue;
		}

		if (food->IsEaten()) 
		{
			continue;
		}

		checkIfIsTooFar = 0;

		foodVector *= 1 / dist;

		float angle = AngleBetween(dir, foodVector);
		if (angle > -m_FOV / 2 && angle < m_FOV / 2) 
		{
			m_Visible.push_back(food);

			int index = (int)((angle + m_FOV / 2) / angleStep);
			//std::cout << index << std::endl;
			float invDist = CalculateInverseDistance(dist);
			float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, index);
			if (invDist > currentDist) 
			{
				m_StateMatrixMemoryArr[currentIndex].Set(0, index, invDist);
			}

		}
		else if (dist < 10.0f) 
		{
			/*if (!m_MemoryOfFoodLocations.empty())
			{
				m_MemoryOfFoodLocations.pop();
			}*/
			//m_MemoryOfFoodLocations.push_back(food);

			//float invDist = CalculateInverseDistance(dist);
			//m_StateMatrixMemoryArr[currentIndex].Set(0, 0, invDist);
			cameClose = true;
		}

		if (dist < 2.0f) 
		{
			/*if (!m_MemoryOfFoodLocations.empty())
			{
				m_MemoryOfFoodLocations.pop();
			}*/

			food->Eat();
			m_CameCloseFood = 50;
			m_FoodEaten++;
			m_Health += 30.0f;
			Reinforcement(m_PositiveQ, m_MemorySize);
			//m_BotBrain.Print();
		}
	}

	//float currentDis1t = m_StateMatrixMemoryArr[currentIndex].Get(0, 0);
	//std::cout << currentDis1t << std::endl;

	if (m_CameCloseFood > 0)
	{
		m_CameCloseFood--;
	}

	if (cameClose && m_CameCloseFood == 0)
	{
		Reinforcement(m_NegativeQClose, m_MemorySize);
		m_CameCloseFood = 50;
	}

	if (m_Location.Magnitude() < 220)
	{
		float magnitude = m_Location.Magnitude();
		float factor = (220 - magnitude) / 220;
		//Reinforcement(m_PositiveQInRadius * factor, m_MemorySize);
	}
	else
	{
		Reinforcement(m_NegativeQOutRadius * m_Location.Magnitude(), m_MemorySize);
	}

	cameClose = false;
	for (QZombie* zombie : zombieList)
	{
		if (!zombie->IsAlive())
		{
			continue;
		}
		
		Vector2 zombieLoc = zombie->GetLocation();
		Vector2 zombieVector = zombieLoc - (m_Location/* - dir * 10*/);
		float dist = (zombieLoc - m_Location).Magnitude();
		
		if (dist > m_MaxDistance)
		{
			continue;
		}
		//else
		//{
		//	zombie->m_Seen = false;
		//}
		
		zombieVector *= 1 / dist;

		float angle = AngleBetween(dir, zombieVector);
		if (angle > -m_FOV && angle < m_FOV)
		{
			m_VisibleZombie.push_back(zombie);

			int index = (int)((angle + m_FOV) / angleStep);
			//std::cout << index << std::endl;
			float invDist = CalculateInverseDistance(dist);
			float currentDist = m_StateMatrixMemoryArr[currentIndex].Get(0, index + 16);
			if (invDist > currentDist)
			{
				m_StateMatrixMemoryArr[currentIndex].Set(0, index + 16, invDist);
			}

			/*if (Dot(dir.GetNormalized(), zombie->GetForward().GetNormalized()) < 0)
			{*/
			if (!zombie->m_Seen)
			{
				Reinforcement(m_NegativeQZombieComingAtYou, m_MemorySize);
				zombie->m_Seen = true;
				//std::cout << "comes" << std::endl;
				//std::cout << zombie->GetIndex() << std::endl;
			}
			//}
		}
		else if (dist < 25.0f)
		{
			cameClose = true;
			Reinforcement(m_PositiveQZombieClose, m_MemorySize);
			zombie->m_Seen = false;
		}
		else
		{
			Reinforcement(m_PositiveQZombieClose, m_MemorySize);
			zombie->m_Seen = false;
		}
		if (dist < 20.0f) {
			m_StartTimer = true;
			//std::cout << "start" << std::endl;
			//Reinforcement(m_NegativeQZombieClose, m_MemorySize);
		}
	}

	if (m_CameCloseZombie > 0)
	{
		m_CameCloseZombie--;
	}

	if (m_StartTimer)
	{
		m_ElapsedSec -= deltaTime;
	}
	else
	{
		m_ElapsedSec = 2;
	}

	if (cameClose && m_ElapsedSec <= 0)
	{
		//Reinforcement(m_PositiveQZombieClose, m_MemorySize);
		//std::cout << "reward" << std::endl;
		m_StartTimer = false;
	}

	//Reinforcement(m_Age * 0.01f, m_MemorySize);

	m_StateMatrixMemoryArr[currentIndex].Set(0, m_NrOfInputs, 1); //bias
	m_StateMatrixMemoryArr[currentIndex].MatrixMultiply(m_BotBrain, m_ActionMatrixMemoryArr[currentIndex]);
	m_ActionMatrixMemoryArr[currentIndex].Sigmoid();

	int r, c;
	m_ActionMatrixMemoryArr[currentIndex].Max(r, c);
	//std::cout << c << std::endl;

	float dAngle = m_SAngle.Get(0, c);
	m_Angle += dAngle * deltaTime;

	m_Speed = 30;
	if (m_ActionMatrixMemoryArr[currentIndex].Get(0, m_NrOfOutputs) == 1)
	{
		m_Speed = 40;
	}
	//std::cout << "30" << std::endl;

	Elite::Vector2 newDir(cos(m_Angle), sin(m_Angle));
	m_Location += newDir * m_Speed * deltaTime;

	m_Health -= 10.f * deltaTime;
	if (m_Health < 0) 
	{
		Dead();
	}
}

void QBot::Render(bool vision, bool details)
{
	if (!m_Alive)
	{
		return;
	}

	Elite::Vector2 dir(cos(m_Angle), sin(m_Angle));
	Elite::Vector2 leftVision(cos(m_Angle + m_FOV / 2), sin(m_Angle + m_FOV / 2));
	Elite::Vector2 rightVision(cos(m_Angle - m_FOV / 2), sin(m_Angle - m_FOV / 2));
	Elite::Vector2 leftVisionEnemies(cos(m_Angle + m_FOV), sin(m_Angle + m_FOV));
	Elite::Vector2 rightVisionEnemies(cos(m_Angle - m_FOV), sin(m_Angle - m_FOV));

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

			//DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * leftVisionEnemies, c);
			//DEBUGRENDERER2D->DrawSegment(m_Location - 10 * dir, m_Location + m_MaxDistance * rightVisionEnemies, c);

			//increase scale of food when is visible
			for (Food* f : m_Visible)
			{
				Vector2 loc = f->GetLocation();
				DEBUGRENDERER2D->DrawCircle(loc, 2, c, 0.5f);
			}
		}

	}

	DEBUGRENDERER2D->DrawString(m_Location, to_string((int)m_Health).c_str());

	if (details)
	{


		//show the inputs and outputs
		for (int i = 0; i < m_NrOfInputs; ++i)
		{
			if (m_StateMatrixMemoryArr[currentIndex].Get(0, i) > 0.0f) {
				DEBUGRENDERER2D->DrawSolidCircle(m_Location - 2.5 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_AliveColor);
			}
			else {
				DEBUGRENDERER2D->DrawSolidCircle(m_Location - 3.0 * dir - perpDir * 2.0f * (i - m_NrOfInputs / 2.0f), 1, perpDir, m_DeadColor);
			}
		}

		//show age and index
		char age[10];
		snprintf(age, 10, "%.1f seconds", m_Age);
		DEBUGRENDERER2D->DrawString(m_Location + m_MaxDistance * dir, age);
		DEBUGRENDERER2D->DrawString(m_Location - m_MaxDistance / 5 * dir, to_string((int)m_Index).c_str());
	}
}

FMatrix* QBot::GetBrain()
{
	//m_BotBrain.Print();
	return &m_BotBrain;
}

void QBot::SetBrain(FMatrix &matrix)
{
	//matrix->Print();
	//m_BotBrain.SetAll(0);
	//m_BotBrain.Add(matrix);
	m_BotBrain.Copy(matrix);
	//m_BotBrain.SetAll(0);
	//matrix->Print();
	//m_BotBrain.Add(*matrix);
	//m_BotBrain.Print();
}

void QBot::Dead(bool zombie)
{
	m_Alive = false;
	if (!zombie)
	{
		Reinforcement(m_NegativeQ, m_MemorySize);	
	}
	else
	{
		Reinforcement(m_NegativeQZombie, m_MemorySize);	
	}
}

bool QBot::IsAlive()
{
	return m_Alive;
}

void QBot::ResetBot() 
{
	// update the bot brain, something went wrong.
	m_Alive = true;
	//Reinforcement(m_NegativeQ, m_MemorySize);
	m_Health = 100.0f;
	m_Location = m_StartLocation;

	m_Age = 0;

	m_TimeOfDeath = 0;
	m_FoodEaten = 0;
}

float QBot::CalculateFitness() const 
{
	return m_FoodEaten + m_TimeOfDeath;
}

//void QBot::MutateMatrix(Generation* gen, Elite::FMatrix& matrix, float mutationRate, float mutationAmplitude) 
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

void QBot::Reinforcement(float factor,int memory)
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


float QBot::CalculateInverseDistance(float realDist) 
{
	// version 1 
	//return m_MaxDistance - realDist;
	// version 2
	float nDist = realDist / m_MaxDistance;
	float invDistSquared = m_MaxDistance / (1 + nDist * nDist);
	return invDistSquared;
}
