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
}

void WorldControllerScript::addAgent(const float* pos)
{
	crowdToolPointer.get()->AddNewAgent(pos);
}

void WorldControllerScript::setPlayer(GameObject* player)
{
	playerUID = player->UUID;
	//convert the player pos to a pointer to float
	float* pos = new float[3];
	pos[0] = player->transform->position.x; pos[1] = player->transform->position.y; pos[2] = player->transform->position.z;
	addAgent(pos);
}

void WorldControllerScript::addEnemy(GameObject* enemy)
{
	//convert the enemy pos to a pointer to float
	float* pos = new float[3];
	pos[0] = enemy->transform->position.x; pos[1] = enemy->transform->position.y; pos[2] = enemy->transform->position.z;
	addAgent(pos);
}

void WorldControllerScript::Update()
{
	//parameter is 0.01 which communicates to crowd update that it has 10ms to do the stuff
	crowdToolPointer.get()->updateCrowd(0.01f);
}
