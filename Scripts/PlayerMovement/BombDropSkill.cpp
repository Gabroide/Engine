#include "BombDropSkill.h"

#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleNavigation.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentBoxTrigger.h"

#include "PlayerState.h"

BombDropSkill::BombDropSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox) :
	MeleeSkill(PM, trigger, attackBox)
{
}


BombDropSkill::~BombDropSkill()
{
	path.clear();
}

void BombDropSkill::Start()
{
	if (player->App->navigation->NavigateTowardsCursor(player->gameobject->transform->position, path,
		math::float3(player->OutOfMeshCorrectionXZ, player->OutOfMeshCorrectionY, player->OutOfMeshCorrectionXZ), intersectionPoint))
	{
		pathIndex = 0;
		player->gameobject->transform->LookAt(intersectionPoint);
		if (bombDropFX)
		{
			bombDropFX->SetActive(true);
		}
		player->ResetCooldown(HUB_BUTTON_Q);
	}

	//Create the hitbox
	boxSize = math::float3(500.f, 500.f, 500.f);
	player->attackBoxTrigger->Enable(true);
	player->attackBoxTrigger->SetBoxSize(boxSize);
}

void BombDropSkill::UseSkill()
{
	if (path.size() > 0 && timer > bombDropPreparationTime)
	{
		math::float3 currentPosition = player->gameobject->transform->GetPosition();
		while (pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < MINIMUM_PATH_DISTANCE)
		{
			pathIndex++;
		}
		if (pathIndex < path.size())
		{
			player->gameobject->transform->LookAt(path[pathIndex]);
			math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
			player->gameobject->transform->SetPosition(currentPosition + bombDropSpeed * direction * player->App->time->gameDeltaTime);
		}
	}

	if (player->attackBoxTrigger != nullptr && !player->attackBoxTrigger->enabled && timer < player->currentState->duration)
	{
		// Update hitbox
		boxPosition = player->transform->up * 100.f; //this front stuff isnt working well when rotating the chicken
		player->attackBoxTrigger->SetBoxPosition(boxPosition.x, boxPosition.y, boxPosition.z + 100.f);
	}
	if (player->attackBoxTrigger != nullptr && player->attackBoxTrigger->enabled && timer > player->currentState->duration)
	{
		player->attackBoxTrigger->Enable(false);
	}
}

void BombDropSkill::CheckInput()
{
	if (timer > player->currentState->duration) //CAN SWITCH?
	{

		if (player->IsUsingSkill())
		{
			player->currentState = (PlayerState*)player->attack;
		}
		else if (player->IsMoving())
		{
			player->currentState = (PlayerState*)player->walk;
		}
		else
		{
			Reset();
		}
	}
}
