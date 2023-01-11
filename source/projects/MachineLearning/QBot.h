#ifndef QBOT_H
#define QBOT_H

#include "framework\EliteMath\EMath.h"
#include "framework\EliteRendering\ERenderingTypes.h"
#include "Food.h"

class QBot
{
public:
	QBot(float x,
		float y,
		float fov,
		float sFov,
		float angle,
		int memorySize,
		int nrInputs,
		int nrOutputs,
		bool useBias,
		int index);

	~QBot();

	void Update(vector<Food*>& food, vector<class QZombie*>& zombieList, float deltaTime);
	void Render(bool vision, bool details);


	//Custom functions
	FMatrix* GetBrain();
	void SetBrain(FMatrix &matrix);
	int GetIndex() { return m_Index; }
	int GetFoodEaten() { return m_FoodEaten; }
	int GetAge() { return m_Age; }
	void Dead(bool zombie = false);
	Vector2 GetLocation() { return m_Location; }

	bool IsAlive();
	void ResetBot();
	float CalculateFitness() const;
	//void MutateMatrix(Generation* gen, Elite::FMatrix& matrix, float mutationRate, float mutationAmplitude);
	void Reinforcement(float factor, int memory);
	float CalculateInverseDistance(float realDist);

private:
	//Custom params
	int m_Index;


	Elite::Vector2 m_Location;
	Elite::Vector2 m_StartLocation;
	float m_Angle;
	float m_FOV;
	float m_MaxDistance = 50.0f;
	float m_SFOV;
	Elite::Color m_AliveColor;
	Elite::Color m_DeadColor;
	float m_Speed = 30.0f;
	float m_Health{ 100.0f };
	bool m_Alive = true;
	float m_Age{ 0.0f };

	// fitness members
	float m_TimeOfDeath = 0;
	int m_FoodEaten = 0;

	vector<Food*> m_Visible;
	vector<QZombie*> m_VisibleZombie;

	int m_NrOfInputs;
	bool m_UseBias;
	int m_NrOfOutputs;
	int m_MemorySize;

	// currentIndex stores the information at the current time.
	// instead of swapping or copying matrices, manipulate the currentIndex to
	// go back in time. currentIndex is updated at the start of the update method
	// so that the render method also has the correct currentIndex. But make sure
	// the matrices at 0 are also filled in, otherwise problems.
	int currentIndex{ -1 };
	Elite::FMatrix* m_StateMatrixMemoryArr;
	Elite::FMatrix* m_ActionMatrixMemoryArr;
	Elite::FMatrix m_BotBrain;
	Elite::FMatrix m_DeltaBotBrain;
	Elite::FMatrix m_SAngle;

	// Q-factors, enable usage for different learning parameters for positive or for negative reinforcement.
	float m_NegativeQ{ -1.f };
	float m_NegativeQZombie{ -0.4f };
	float m_NegativeQZombieClose{ -0.2f };
	float m_NegativeQClose{ -0.8f };
	float m_PositiveQ{ 1.2f };
	float m_PositiveQZombieClose{ 0.2f };
	float m_PositiveQInRadius{ 0.01f };
	float m_PositiveQOutRadius{ 1.5f };
	int m_CameCloseFood{ 0 };
	int m_CameCloseZombie{ 0 };

};

#endif