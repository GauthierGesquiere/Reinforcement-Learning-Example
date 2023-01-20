/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Koen Samyn
/*=============================================================================*/
#ifndef DYNAMIC_Q_LEARNING
#define DYNAMIC_Q_LEARNING

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------

//custom includes
#include <fstream>


#include "QBot.h"
#include "QZombie.h"

class DynamicQLearning final
{
public:
	DynamicQLearning(int nrOfFood, int memorySize, int nrOfInputs, int nrOfOutputs, bool bias );
	~DynamicQLearning();

	void Update(float deltaTime, bool render);
	void Render(float deltaTime);

	void SaveBeginData(string s) const;
	void SaveEndGenData(string s, QBot* bot) const;

private:
	//Custom params
	int m_GenCount = 1;

	float m_ElapsedSec = 0;

	int m_BestGen = 1;
	int m_MostFoodEaten = 0;
	int m_BotsAlive = -1;
	float m_BestTimeAlive = 0;

	int m_Reached600 = 0;
	int m_NotReached600 = 0;

	const int m_NrOfBots = 50;
	const int m_NrOfZombies = 10;
	const int m_NrOfFood;
	bool m_AssignLastBotAlive = true;
	bool m_SaveAtEnd;
	vector<QBot*> m_Bots;
	vector<QZombie*> m_Zombies;

	const string m_DirectoryName = "V4Files";
	const string m_FileName = "BasicV";

	string m_CurrentFilePath;

	bool m_NextBot;
	bool m_PreviousBot;
	int m_CameraIndex;

	bool m_RenderDetails = true;
	bool m_RenderVision = true;
	bool m_RenderOneBot = true;

	bool m_EndGeneration = false;

	void ResetEnvironment();
	void ReadMatrixFromFile(string s);

	static const int MEMORY_SIZE = 200;
	const float m_LearningCurve = 0.4f;

	int m_MemorySize;
	int m_NrOfInputs;
	int m_NrOfOutputs;
	bool m_UseBias;

	// currentIndex stores the information at the current time.
	// instead of swapping or copying matrices, manipulate the currentIndex to
	// go back in time.
	//int currentIndex = 0;
	QBot* m_pLastBotAlive{ 0 };

	// environment
	vector<Food*> m_Foodstuff;
};

#endif
