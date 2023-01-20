//=== General Includes ===
#include "stdafx.h"
#include "DynamicQLearning.h"
#include "Food.h"

//custom includes
#include <filesystem>

DynamicQLearning::DynamicQLearning(int nrOfFood, int memorySize, int nrOfInputs, int nrOfOutputs, bool bias)
	:m_MemorySize(memorySize),
	m_NrOfInputs(nrOfInputs),
	m_NrOfOutputs(nrOfOutputs),
	m_UseBias(bias),
	m_NrOfFood(nrOfFood)
{
	//Check if a directory exist
	struct stat sb;
	string s;

	//Calls the function with path as argument
	//If the file/directory exists at the path returns 0
	//If block executes if path exists
	if (stat(m_DirectoryName.c_str(), &sb) != 0)
	{
		s = "mkdir " + m_DirectoryName;
		system(s.c_str());
	}
	struct stat sb1;
	//add another file
	int counter = 0;
	while (true)
	{
		m_CurrentFilePath = s = m_DirectoryName + "/" + m_FileName + std::to_string(counter) + ".csv";
		if (stat(s.c_str(), &sb1) == 0 && !(sb1.st_mode & S_IFDIR))
		{
			counter++;
		}
		else
		{
			ofstream myfile(s);
			break;
		}
	}

	SaveBeginData(s);

	//assign all the bots random values
	for (int i = 0; i < m_NrOfBots; ++i)
	{
		float startx = Elite::randomFloat(0.0f, 50.0f);
		float starty = Elite::randomFloat(-50.0f, 50.0f);
		float startAngle = Elite::randomFloat(0, float(M_PI) * 2);

		QBot* bot = new QBot(startx, starty, float(M_PI) / 3, 2 * float(M_PI), startAngle, MEMORY_SIZE, m_NrOfInputs, m_NrOfOutputs, m_UseBias, i);
		m_Bots.push_back(bot);
	}

	//ReadMatrixFromFile("matrix.txt");

	//create zombie bots
	for (int i = 0; i < m_NrOfZombies; ++i)
	{
		float startx = Elite::randomFloat(-200.0f, 200.0f);
		float starty = Elite::randomFloat(-200.0f, 200.0f);
		float startAngle = Elite::randomFloat(0, float(M_PI) * 2);

		QZombie* bot = new QZombie(startx, starty, float(M_PI) / 3, 2 * float(M_PI), startAngle, 50, 16, m_NrOfOutputs, m_UseBias, i);
		m_Zombies.push_back(bot);
	}

	//Initialization of your application. If you want access to the physics world you will need to store it yourself.
	for (int i = 0; i < nrOfFood; ++i)
	{
		float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		angle *= 2 * float(M_PI);
		float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		dist *= 200;
		dist += 20;
		Food* f = new Food(dist * cos(angle), dist * sin(angle));
		m_Foodstuff.push_back(f);
	}
}

DynamicQLearning::~DynamicQLearning() 
{
	if (m_SaveAtEnd)
	{
		for (QBot*& pBot : m_Bots)
		{
			SaveEndGenData(m_CurrentFilePath, pBot);
		}
	}

	for (QBot*& pBot : m_Bots)
	{
		SAFE_DELETE(pBot);
	}

	for (QZombie*& pZombie : m_Zombies)
	{
		SAFE_DELETE(pZombie);
	}

	for (Food* &pFood : m_Foodstuff)
	{
		SAFE_DELETE(pFood);
	}
}
void DynamicQLearning::Update(float deltaTime, bool render) 
{
	m_ElapsedSec += deltaTime;
	//Make var to check how many bots are still alive
	int counter = 0;

	//Make a temp var to set the last bot
	QBot* tempBot = m_Bots[0];


	//Check if we take the worst bot what will happen


	//for (QBot* pBot : m_Bots)
	//{
	//	//Do Update
	//	pBot->Update(m_Foodstuff, m_Zombies, deltaTime);

	//	//Check how many bots are alive
	//	if (!pBot->IsAlive())
	//	{
	//		m_BotsAlive = -1;
	//		m_pLastBotAlive = pBot;
	//		
	//		m_MostFoodEaten = m_pLastBotAlive->GetFoodEaten();
	//		m_BestGen = m_GenCount;
	//		m_BestTimeAlive = m_ElapsedSec;

	//		ResetEnvironment();
	//	}
	//}

	///////////////////////////////////////

	

	//Go over all bots and do update
	for (QBot* pBot : m_Bots)
	{
		//Do Update
		pBot->Update(m_Foodstuff, m_Zombies, deltaTime);

		//Check how many bots are alive
		if (pBot->IsAlive())
		{
			counter++;
			tempBot = pBot;
		}
	}

	//switch to another bot when current bot dies
	for (QBot* pBot : m_Bots)
	{
		if (counter > 0 && !pBot->IsAlive())
		{
			if (pBot == m_Bots[m_CameraIndex])
			{
				while (true)
				{
					m_CameraIndex++;
					if (m_CameraIndex >= m_NrOfBots)
					{
						m_CameraIndex = 0;
					}
					if (m_Bots[m_CameraIndex]->IsAlive())
					{
						break;
					}
				}
			}
		}
	}

	//if the last botvar may be set and there is only 1 bot left set the lastbot that is alive
	if (counter <= 1 && m_AssignLastBotAlive)
	{
		m_AssignLastBotAlive = false;

		m_pLastBotAlive = tempBot;
	}
	//if there are 0 bots alive and the last bot has been set then reset the environment
	else if (counter <= 0 && !m_AssignLastBotAlive)
	{
		if (m_BotsAlive <= 1 && m_BestTimeAlive < m_ElapsedSec)
		{
			m_BotsAlive = 1;
			
			m_MostFoodEaten = m_pLastBotAlive->GetFoodEaten();
			m_BestGen = m_GenCount;
			m_BestTimeAlive = m_ElapsedSec;
		}

		m_NotReached600++;

		ResetEnvironment();
	}
	if (m_EndGeneration)
	{
		QBot* bestAgeBot = m_Bots[0];
		for (QBot* pBot : m_Bots)
		{
			int age = pBot->GetAge();
			int bestAge = bestAgeBot->GetAge();
			if (age > bestAge)
				bestAgeBot = pBot;
		}

		m_pLastBotAlive = bestAgeBot;

		ResetEnvironment();
	}
	if (m_ElapsedSec >= 600.0f)
	{
		QBot* bestAgeBot = m_Bots[0];
		for (QBot* pBot : m_Bots)
		{
			int age = pBot->GetAge();
			int bestAge = bestAgeBot->GetAge();
			if (age > bestAge)
				bestAgeBot = pBot;
		}

		if (counter >= m_BotsAlive)
		{
			m_BotsAlive = counter;

			m_MostFoodEaten = bestAgeBot->GetFoodEaten();
			m_BestGen = m_GenCount;
			m_BestTimeAlive = m_ElapsedSec;
		}

		m_Reached600++;

		m_pLastBotAlive = bestAgeBot;

		ResetEnvironment();
	}

	int zombieCount = 0;
	for (QZombie*& pZombie : m_Zombies)
	{
		pZombie->Update(m_Bots, deltaTime);

		/*if (!pZombie->IsAlive())
		{
			zombieCount++;
		}*/
	}

	/*if (zombieCount >= m_Zombies.size())
	{
		if (counter > 0)
		{
			m_pLastBotAlive = tempBot;
			ResetEnvironment();
		}
		else
		{
			for (QBot* pBot : m_Bots)
			{
				pBot->ResetBot();
			}

			for (QZombie*& pZombie : m_Zombies)
			{
				pZombie->ResetBot();
			}
		}
	}*/

	for (Food* pFood : m_Foodstuff) 
	{
		pFood->Update();
	}

	//UI
	if (render)
	{
		//Setup
		int const menuWidth = 235;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Params", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Checkbox("Save At End", &m_SaveAtEnd);
		string s = "Alive: " + std::to_string(counter);
		ImGui::Text(s.c_str());
		s = "Generation: " + std::to_string(m_GenCount);
		ImGui::Text(s.c_str());
		s = "Memory Size: " + std::to_string(MEMORY_SIZE);
		ImGui::Text(s.c_str());

		s = "Learning Curve: " + std::to_string(m_LearningCurve);
		ImGui::Text(s.c_str());

		////if there are no bots alive
		//if (counter <= 0)
		//{
		//	return;
		//}

		ImGui::Text("BOTS");
		ImGui::Indent();
		if (ImGui::Button("Next"))
		{
			while(true)
			{
				m_CameraIndex++;
				if (m_CameraIndex >= m_NrOfBots)
				{
					m_CameraIndex = 0;
				}
				if (m_Bots[m_CameraIndex]->IsAlive())
				{
					break;
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Previous"))
		{
			while (true)
			{
				m_CameraIndex--;
				if (m_CameraIndex < 0)
				{
					m_CameraIndex = m_NrOfBots - 1;
				}
				if (m_Bots[m_CameraIndex]->IsAlive())
				{
					break;
				}
			}
		}

		ImGui::Checkbox("Render Details", &m_RenderDetails);
		ImGui::Checkbox("Render Vision", &m_RenderVision);
		ImGui::Spacing();
		ImGui::Checkbox("Only Render One Bot", &m_RenderOneBot);


		m_EndGeneration = ImGui::Button("End Generation");

		ImGui::Unindent();
		ImGui::Spacing();
		ImGui::Text("Best Gen: "); ImGui::SameLine();  ImGui::Text(std::to_string(m_BestGen).c_str());
		ImGui::Text("Most Food Eaten: "); ImGui::SameLine();  ImGui::Text(std::to_string(m_MostFoodEaten).c_str());
		ImGui::Text("Most Bots alive: "); ImGui::SameLine();  ImGui::Text(std::to_string(m_BotsAlive).c_str());
		ImGui::Text("Time Survived "); ImGui::SameLine();  ImGui::Text(std::to_string(m_BestTimeAlive).c_str());

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Times reached 600sec: "); ImGui::SameLine();  ImGui::Text(std::to_string(m_Reached600).c_str());
		ImGui::Text("Times not reached 600sec:"); ImGui::SameLine();  ImGui::Text(std::to_string(m_NotReached600).c_str());
		

		DEBUGRENDERER2D->GetActiveCamera()->SetCenter(m_Bots[m_CameraIndex]->GetLocation());

		ImGui::End();
	}

}

void DynamicQLearning::Render(float deltaTime)
{
	for (QBot* pBot : m_Bots)
	{
		if (m_RenderOneBot)
		{
			if (pBot != m_Bots[m_CameraIndex])
			{
				pBot->Render(false, false);
			}
			else
			{
				pBot->Render(m_RenderVision, m_RenderDetails);
			}
		}
		else
		{
			pBot->Render(m_RenderVision, m_RenderDetails);
		}
	}
	
	for (QZombie* pZombie : m_Zombies)
	{
		if (m_RenderOneBot)
		{
			pZombie->Render(false, false);

		}
		else
		{
			pZombie->Render(m_RenderVision, m_RenderDetails);
		}
	}

	for (Food* pFood : m_Foodstuff) 
	{
		pFood->Render();
	}
}

void DynamicQLearning::SaveBeginData(string s) const
{
	ofstream myFile;
	myFile.open(s);
	myFile << "NAME: " << s << std::endl << std::endl;
	myFile << "Nr of Bots:;" << m_NrOfBots << std::endl;
	myFile << "Nr of food:;" << m_NrOfFood << std::endl;
	myFile << "Nr of inputs:;" << m_NrOfInputs << std::endl;
	myFile << "Nr of outputs:;" << m_NrOfOutputs << std::endl;
	myFile << "Memory size:;" << MEMORY_SIZE << std::endl << std::endl;
	myFile.close();
}


void DynamicQLearning::SaveEndGenData(string s, QBot* bot) const
{
	ofstream myFile;
	myFile.open(s, std::ios_base::app);
	myFile << "Genaration:;" << m_GenCount << std::endl;
	myFile << "Survivor index:;" << bot->GetIndex() << std::endl;
	myFile << "Nr of food eaten:;" << bot->GetFoodEaten() << std::endl;
	myFile << "Age:;" << bot->GetAge() << std::endl;
	myFile << "The brain:;" << std::endl << std::endl;

	FMatrix* matrix = bot->GetBrain();
	for (int c_row = 0; c_row < matrix->GetNrOfRows(); ++c_row) 
	{
		for (int c_column = 0; c_column < matrix->GetNrOfColumns(); ++c_column) 
		{
			float value = matrix->Get(c_row, c_column);
			myFile << value << ";";
		}
		myFile << std::endl;
	}
	myFile << std::endl;
	myFile.close();

}

void DynamicQLearning::ResetEnvironment()
{
	std::cout << m_ElapsedSec << std::endl;


	//Reset Whole environment
	//Get The brain from the last survivor
	FMatrix* bestBrain = m_pLastBotAlive->GetBrain();

	m_ElapsedSec = 0.0f;

	SaveEndGenData(m_CurrentFilePath, m_pLastBotAlive);

	m_GenCount++;

	//Loop over all bots
	for (int i = 0; i < m_NrOfBots; ++i)
	{
		//randomize the brains a bit to have a more faster learning cycle
		//dont include last one so the brain can never be worse
		/*if (i < m_NrOfBots - 1)
		{
			FMatrix randomizedMatrix = FMatrix(bestBrain->GetNrOfRows(), bestBrain->GetNrOfColumns());
			randomizedMatrix.Randomize(-m_LearningCurve, m_LearningCurve);
			bestBrain->Add(randomizedMatrix);
		}*/

		m_Bots[i]->SetBrain(*bestBrain);
		m_Bots[i]->ResetBot();
	}

	for (QZombie* pZombie : m_Zombies)
	{
		pZombie->ResetBot();
	}

	m_pLastBotAlive = nullptr;
	m_AssignLastBotAlive = true;
}

void DynamicQLearning::ReadMatrixFromFile(string s)
{
	int n, m;
	ifstream myfile;
	myfile.open(s);
	myfile >> n >> m;

	Elite::FMatrix test = Elite::FMatrix(n, m);
	for (int i = 0; i < n; i++) 
	{
		for (int j = 0; j < m; j++) 
		{
			float value;
			myfile >> value;
			test.Set(i, j, value);
		}
	}



	for (int i = 0; i < m_NrOfBots; ++i)
	{
		//m_Bots[i]->GetBrain()->Print();
		m_Bots[i]->SetBrain(test);
		//m_Bots[i]->GetBrain()->Print();
	}
}
