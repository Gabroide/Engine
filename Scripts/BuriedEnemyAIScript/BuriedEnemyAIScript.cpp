#include "BuriedEnemyAIScript.h"

#include "EnemyControllerScript.h"
#include "EnemyStateChase.h"
#include "EnemyStateAttack.h"
#include "EnemyStateCooldown.h"
#include "EnemyStateDeath.h"
#include "EnemyStateHidden.h"
#include "EnemyStateHide.h"
#include "EnemyStateShowUp.h"
#include "EnemyStateRelocate.h"
#include "EnemyStateReturnToStart.h"

#include "GameObject.h"
#include "ComponentAnimation.h"

BuriedEnemyAIScript_API Script* CreateScript()
{
	BuriedEnemyAIScript* instance = new BuriedEnemyAIScript;
	return instance;
}

void BuriedEnemyAIScript::Awake()
{
	// Look for Enemy Controller Script of the enemy
	enemyController = gameobject->GetComponent<EnemyControllerScript>();
	if (enemyController == nullptr)
	{
		LOG("The GameObject %s has no Enemy Controller Script component attached \n", gameobject->name);
	}
}

void BuriedEnemyAIScript::Start()
{
	//Create states
	enemyStates.reserve(9);
	enemyStates.push_back(attack = new EnemyStateAttack(this));
	enemyStates.push_back(cooldown = new EnemyStateCooldown(this));
	enemyStates.push_back(hide = new EnemyStateHide(this));
	enemyStates.push_back(hidden = new EnemyStateHidden(this));
	enemyStates.push_back(showUp = new EnemyStateShowUp(this));
	enemyStates.push_back(returnToStart = new EnemyStateReturnToStart(this));
	enemyStates.push_back(chase = new EnemyStateChase(this));
	enemyStates.push_back(death = new EnemyStateDeath(this));
	enemyStates.push_back(relocate = new EnemyStateRelocate(this));


}

void BuriedEnemyAIScript::Update()
{
	EnemyState* previous = currentState;

	currentState->HandleIA();
	currentState->Update();
	
	CheckStates(previous);
}

void BuriedEnemyAIScript::Expose(ImGuiContext * context)
{
}

void BuriedEnemyAIScript::Serialize(JSON_value * json) const
{
}

void BuriedEnemyAIScript::DeSerialize(JSON_value * json)
{
}

void BuriedEnemyAIScript::CheckStates(EnemyState* previous)
{
	if (previous != currentState)
	{
		previous->ResetTimer();

		previous->Exit();
		currentState->Enter();

		if (enemyController->anim != nullptr)
		{
			enemyController->anim->SendTriggerToStateMachine(currentState->trigger.c_str());
		}

		currentState->duration = enemyController->anim->GetDurationFromClip();
	}
}
