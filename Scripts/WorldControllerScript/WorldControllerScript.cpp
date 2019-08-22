#include "WorldControllerScript.h"

#include "Application.h"
#include "GameObject.h"
#include "ModuleNavigation.h"
#include "ComponentTransform.h"

#include "Globals.h"

WorldControllerScript_API Script* CreateScript()
{
	WorldControllerScript* instance = new WorldControllerScript;
	return instance;
}

WorldControllerScript::WorldControllerScript()
{
}

WorldControllerScript::~WorldControllerScript()
{
}

void WorldControllerScript::resetWorldAgents()
{
	crowdToolPointer.reset();//delete smart pointer calling the destructor
	crowdToolPointer = std::make_unique<crowdTool>();//create a new instance
	//reset our map aswell
	objectAgentIndexMap.clear();
}

int WorldControllerScript::addAgent(const math::float3* pos)
{
	return crowdToolPointer.get()->AddNewAgent(pos);
}

void WorldControllerScript::setPlayer(GameObject* player)
{
	playerUID = player->UUID;
	int newAgentId = addAgent(&player->transform->position);
	//once we created the agent, save the info in our map
	objectAgentIndexMap[player->UUID] = newAgentId;
}

void WorldControllerScript::addEnemy(GameObject* enemy)
{
	int newAgentId = addAgent(&enemy->transform->position);
	//once we created the agent, save the info in our map
	objectAgentIndexMap[enemy->UUID] = newAgentId;
}

void WorldControllerScript::moveRequest(GameObject* agent)
{
	//gets the agent id from the map we have 
}

void WorldControllerScript::Update()
{
	//parameter is 0.01 which communicates to crowd update that it has 10ms to do the stuff
	crowdToolPointer.get()->UpdateCrowd(0.01f);
}
