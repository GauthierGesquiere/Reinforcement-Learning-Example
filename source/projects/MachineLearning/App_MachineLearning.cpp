//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
using namespace Elite;

//Includes
#include "App_MachineLearning.h"

//Statics

//Destructor
App_MachineLearning::~App_MachineLearning()
{
	SAFE_DELETE(m_pDynamicQEnv);
}

//Functions
void App_MachineLearning::Start()
{
	srand(time(NULL));

	//Initialization of your application. 
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(75.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(50,50));

	m_pDynamicQEnv = new DynamicQLearning(200, 100, 48, 6, true);
}

void App_MachineLearning::Update(float deltaTime)
{
	/*if (m_FastForward)
	{
		for (size_t i = 0; i < 600000; i++)
		{
			m_pDynamicQEnv->Update(0.01f, !m_FastForward);
		}

		m_FastForward = false;
		m_FirstIteration = true;
	}
	else if (m_FirstIteration)
	{
		m_pDynamicQEnv->Update(0.01f, !m_FastForward);
		m_FirstIteration = false;
	}
	else
	{*/
		m_pDynamicQEnv->Update(deltaTime, !m_FastForward);
	//}

}

void App_MachineLearning::Render(float deltaTime) const
{
	/*if (!m_FastForward)
	{*/
		m_pDynamicQEnv->Render(deltaTime);
	//}
}


