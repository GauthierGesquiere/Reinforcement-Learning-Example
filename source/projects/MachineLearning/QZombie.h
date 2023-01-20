#ifndef QZOMBIE_H
#define QZOMBIE_H

#include "framework\EliteMath\EMath.h"
#include "framework\EliteRendering\ERenderingTypes.h"
#include "Food.h"
#include "QBot.h"

class QZombie
{
public:
	QZombie(float x,
		float y,
		float fov,
		float sFov,
		float angle,
		int memorySize,
		int nrInputs,
		int nrOutputs,
		bool useBias,
		int index);

	~QZombie();

	void Update(vector<QBot*>& bot, float deltaTime);
	void Render(bool vision, bool details);


	//Custom functions
	FMatrix* GetBrain();
	void SetBrain(FMatrix* matrix);
	int GetIndex() { return m_Index; }
	int GetFoodEaten() { return m_BotEaten; }
	int GetAge() { return m_Age; }
	Vector2 GetLocation() { return m_Location; }
	Vector2 GetForward() { return Vector2(cos(m_Angle), sin(m_Angle)); }

	bool m_Seen = false;

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
	int m_BotEaten = 0;

	vector<QBot*> m_Visible;

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
	float m_NegativeQ{ -0.2f };
	float m_NegativeQClose{ -0.1f };
	float m_PositiveQ{ 0.1f };
	int m_CameCloseCounter{ 0 };

};

#endif