#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"
#include "CellSpace/CellSpace.h"

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 5.0f }
	, m_NrOfNeighbors{0}
{
	// TODO: initialize the flock and the memory pool
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pAlignmenthBehavior = new Alignment(this);
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();

	vector<BlendedSteering::WeightedBehavior>WeightedSteerinBehaviourFlocking;

	WeightedSteerinBehaviourFlocking.push_back({ m_pCohesionBehavior, 0.0f });
	WeightedSteerinBehaviourFlocking.push_back({ m_pSeparationBehavior, 0.0f });
	WeightedSteerinBehaviourFlocking.push_back({ m_pAlignmenthBehavior, 0.0f });
	WeightedSteerinBehaviourFlocking.push_back({ m_pSeekBehavior, 0.0f });
	WeightedSteerinBehaviourFlocking.push_back({ m_pWanderBehavior, 0.0f });

	m_pBlendedSteering = new BlendedSteering(WeightedSteerinBehaviourFlocking);

	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior , m_pBlendedSteering });

	for (auto i = 0; i < m_FlockSize; i++)
	{
		m_Agents.push_back(new SteeringAgent);
		m_Agents[i]->SetPosition({ float(rand() % int(m_WorldSize)), float(rand() % int(m_WorldSize)) });
		m_Agents[i]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[i]->SetMass(1.0f);
		m_Agents[i]->SetMaxAngularSpeed(25.f);
		m_Agents[i]->SetMaxLinearSpeed(55.f);
		m_Agents[i]->SetAutoOrient(true);
	}
	
	m_pAgentToEvade = new SteeringAgent;
	m_pAgentToEvade->SetSteeringBehavior(m_pWanderBehavior);
	m_pAgentToEvade->SetPosition({ float(rand() % int(m_WorldSize)), float(rand() % int(m_WorldSize)) });

	m_pAgentToEvade->SetBodyColor({ 1,0,0,1 });

#ifdef USE_SPACE_PARTITIONING
	m_OldPositions.resize(m_Agents.size());
	m_pCellSpace = new CellSpace(this);
#endif
}

Flock::~Flock()
{
	// TODO: clean up any additional data
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pAlignmenthBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();
#ifdef USE_SPACE_PARTITIONING
	SAFE_DELETE(m_pCellSpace);
#endif
	SAFE_DELETE(m_pAgentToEvade);
}

void Flock::Update(float deltaT)
{
	int counter = 0;
	// TODO: update the flock
	for (auto pAgent : m_Agents)
	{

#ifdef USE_SPACE_PARTITIONING
		// Spacial partitioning
		m_pCellSpace->UpdateAgentCell(deltaT, pAgent, m_OldPositions[counter]);
		m_pCellSpace->RegisterNeighbors(pAgent);
		m_OldPositions[counter] = pAgent->GetPosition();
		counter++;
#else
		//No Spacial partitioning
		RegisterNeighbors(pAgent);
#endif
		pAgent->Update(deltaT);

		if (m_TrimWorld)
		{
			pAgent->TrimToWorld(Elite::Vector2(0, 0), Elite::Vector2(m_WorldSize, m_WorldSize));
			m_pAgentToEvade->TrimToWorld(Elite::Vector2(0, 0), Elite::Vector2(m_WorldSize, m_WorldSize));
		}
		if (pAgent->CanRenderBehavior())
		{
			//look at the neighbors for the first agent
			/*if (m_Agents[0] == pAgent)
			{
				DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, Elite::Color{ 1, 0, 0, 1 }, 0.0f);
				for (auto pOtherAgent : m_Agents)
				{
					Elite::Vector2 VectorBetweenPoints;
					VectorBetweenPoints = CalcVector(pAgent->GetPosition(), pOtherAgent->GetPosition());

					float distanceBetween2Points = sqrt(VectorBetweenPoints.x * VectorBetweenPoints.x + VectorBetweenPoints.y * VectorBetweenPoints.y);

					if (distanceBetween2Points < m_NeighborhoodRadius && distanceBetween2Points != 0.0f)
					{
						pOtherAgent->SetBodyColor({ 0,0,1,1 });
					}
					else
					{
						pOtherAgent->SetBodyColor({ 1,1,0,1 });
					}
				}
			}*/
		}
		
		
		m_NrOfNeighbors = 0.0f;
		m_Neighbors.clear();
	}

	m_pAgentToEvade->Update(deltaT);
}

void Flock::Render(float deltaT)
{
#ifdef USE_SPACE_PARTITIONING
#else
	// TODO: render the flock
	for (auto pAgent : m_Agents)
	{
		pAgent->Render(deltaT);
	}
	m_pAgentToEvade->Render(deltaT);
#endif
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here
	ImGui::Text("Behavior Weights");
	ImGui::Spacing();

	if (ImGui::Button("Enable Rendering")) 
	{
		m_Agents[0]->SetRenderBehavior(EnableDebugRendering);
		EnableDebugRendering = !EnableDebugRendering;
	}

	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->GetWeightedBehaviorsRef()[0].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->GetWeightedBehaviorsRef()[1].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Alignment", &m_pBlendedSteering->GetWeightedBehaviorsRef()[2].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->GetWeightedBehaviorsRef()[3].weight, 0.f, 1.f, "%.2");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->GetWeightedBehaviorsRef()[4].weight, 0.f, 1.f, "%.2");

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}
#ifdef USE_SPACE_PARTITIONING
#else
void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	for (auto pOtherAgent : m_Agents)
	{
		Elite::Vector2 VectorBetweenPoints;
		VectorBetweenPoints = CalcVector(pAgent->GetPosition(), pOtherAgent->GetPosition());

		float distanceBetween2Points = sqrt(VectorBetweenPoints.x * VectorBetweenPoints.x + VectorBetweenPoints.y * VectorBetweenPoints.y);

		if (distanceBetween2Points < m_NeighborhoodRadius && distanceBetween2Points != 0.0f)
		{
			m_NrOfNeighbors++;
			m_Neighbors.push_back(pOtherAgent);
		}
	}
}
#endif


void Flock::SetSeekTarget(TargetData target)
{
	// TODO: set target for Seek behavior
	m_pSeekBehavior->SetTarget(target);
	m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

