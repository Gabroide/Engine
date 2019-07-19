#include "FearController.h"

#include "BasicEnemyAIScript.h"
#include "EnemyControllerScript.h"

FearController_API Script* CreateScript()
{
	FearController* instance = new FearController;
	return instance;
}

void FearController::Awake()
{
}

void FearController::Start()
{
}

void FearController::Update()
{
	if (IsSomeoneDead())
	{
		LaunchDeathEvent();
	}
}

void FearController::Expose(ImGuiContext * context)
{
}

void FearController::Serialize(JSON_value * json) const
{
}

void FearController::DeSerialize(JSON_value * json)
{
}

void FearController::LaunchDeathEvent()
{
	//maybe we should check positions

	if(enemy1 != nullptr)
		enemy1->scared = true;
	if(enemy2 != nullptr)
		enemy2->scared = true;
	if(enemy3 != nullptr)
		enemy3->scared = true;
}

bool FearController::IsSomeoneDead()
{
	bool enemy1Dead = false;
	if(enemy1 != nullptr)
	{

	}
	return (enemy1->enemyController->IsDead() || enemy2->enemyController->IsDead() 
		|| enemy3->enemyController->IsDead());
}
