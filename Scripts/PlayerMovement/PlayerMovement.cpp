#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "PlayerState.h"
#include "PlayerStateFirstAttack.h"
#include "PlayerStateSecondAttack.h"
#include "PlayerStateThirdAttack.h"
#include "PlayerStateDash.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDeath.h"
#include "PlayerStateUppercut.h"

#include "ComponentAnimation.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"
#include "GameObject.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

#define CLOSE_ENOUGH 400.0f

PlayerMovement_API Script* CreateScript()
{
	PlayerMovement* instance = new PlayerMovement;
	return instance;
}

void PlayerMovement::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);

	//Exposing durations this should access to every class instead of allocating them in PlayerMovement, but for now scripts don't allow it
	ImGui::DragFloat("Walking speed", &walkingSpeed, 0.01f, 10.f, 500.0f);
	ImGui::DragFloat("Dash duration", &dashDuration, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat("First attack duration", &firstAttackDuration, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat("Second attack duration", &secondAttackDuration, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat("Third attack duration", &thirdAttackDuration, 0.01f, 0.1f, 5.0f);
	ImGui::DragFloat("UpperCut", &uppercutDuration, 0.01f, 0.1f, 5.0f);
}

void PlayerMovement::Start()
{
	playerStates.reserve(8);

	playerStates.push_back(walk = new PlayerStateWalk(this));
	playerStates.push_back(firstAttack = new PlayerStateFirstAttack(this));
	playerStates.push_back(secondAttack = new PlayerStateSecondAttack(this));
	playerStates.push_back(thirdAttack = new PlayerStateThirdAttack(this));
	playerStates.push_back(idle = new PlayerStateIdle(this));
	playerStates.push_back(death = new PlayerStateDeath(this));
	playerStates.push_back(uppercut = new PlayerStateUppercut(this));
	playerStates.push_back(dash = new PlayerStateDash(this));
	Appl = App;

	currentState = idle;

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name);
	}

	boxTrigger = gameobject->GetComponent<ComponentBoxTrigger>();
	if (boxTrigger == nullptr)
	{
		LOG("The GameObject %s has no boxTrigger component attached \n", gameobject->name);
	}

	transform = gameobject->GetComponent<ComponentTransform>();
	if (transform == nullptr)
	{
		LOG("The GameObject %s has no transform component attached \n", gameobject->name);
	}
	LOG("Started player movement script");
}
void PlayerMovement::Update()
{
	if (health <= 0.f)
	{
		currentState = (PlayerState*)death;
		return;
	}

	PlayerState* previous = currentState;

	//Check input here and update the state!
	if (currentState != death)
	{
		currentState->UpdateTimer();

		currentState->CheckInput();

		//if previous and current are different the functions Exit() and Enter() are called
		CheckStates(previous, currentState);

		currentState->Update();
	}
		
	//Check for changes in the state to send triggers to animation SM
	

	//if (isPlayerDead) return;

	//PlayerState previous = playerState;
	//if (playerState == PlayerState::ATTACK)
	//{
	//	attackTimer += App->time->gameDeltaTime;
	//	if (attackTimer >= attackDuration)
	//	{
	//		attackTimer = 0.0f;
	//		playerState = PlayerState::IDLE;
	//	}
	//}
	//if (App->input->GetMouseButtonDown(3) == KEY_DOWN) //RIGHT BUTTON
	//{
	//	math::float3 intersectionPoint = math::float3::inf;
	//	if (App->scene->Intersects(intersectionPoint, "floor"))
	//	{
	//		App->navigation->FindPath(gameobject->transform->position, intersectionPoint, path);
	//		pathIndex = 0;
	//	}
	//}
	//if (path.size() > 0)
	//{
	//	math::float3 currentPosition = gameobject->transform->GetPosition();
	//	while(pathIndex < path.size() && currentPosition.DistanceSq(path[pathIndex]) < CLOSE_ENOUGH)
	//	{
	//		pathIndex++;
	//	}
	//	if (pathIndex < path.size())
	//	{
	//		gameobject->transform->LookAt(path[pathIndex]);
	//		math::float3 direction = (path[pathIndex] - currentPosition).Normalized();
	//		gameobject->transform->SetPosition(currentPosition + speed*direction*App->time->gameDeltaTime);
	//		playerState = PlayerState::WALK;
	//	}
	//	else
	//	{
	//		playerState = PlayerState::IDLE;
	//	}
	//}
	//else if (playerState != PlayerState::ATTACK)
	//{
	//	playerState = PlayerState::IDLE;
	//}

	//if (App->input->GetMouseButtonDown(1) == KEY_DOWN)
	//{
	//	pathIndex = 0;
	//	path.clear();
	//	playerState = PlayerState::ATTACK;
	//	math::float3 attackPosition;
	//	if (App->scene->Intersects(attackPosition, "floor"))
	//	{
	//		gameobject->transform->LookAt(attackPosition);
	//	}
	//}
	//CheckState(previous, playerState);
}

PlayerMovement_API void PlayerMovement::Damage(float amount)
{
	health -= amount;
	if (health < 0)
	{
		isPlayerDead = true;
	}
}

void PlayerMovement::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("walkingSpeed", walkingSpeed);
	json->AddFloat("firstAttackDuration", firstAttackDuration);
	json->AddFloat("secondAttackDuration", secondAttackDuration);
	json->AddFloat("thirdAttackDuration", thirdAttackDuration);
	json->AddFloat("uppercutDuration", uppercutDuration);
	json->AddFloat("dashDuration", dashDuration);
}

void PlayerMovement::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	walkingSpeed = json->GetFloat("walkingSpeed", 100.0f);
	firstAttackDuration = json->GetFloat("firstAttackDuration");
	secondAttackDuration = json->GetFloat("secondAttackDuration");
	thirdAttackDuration = json->GetFloat("thirdAttackDuration");
	uppercutDuration = json->GetFloat("uppercutDuration");
	dashDuration = json->GetFloat("dashDuration");
}

bool PlayerMovement::IsAtacking()
{
	return App->input->GetMouseButtonDown(1) == KEY_DOWN; //Left button
}

bool PlayerMovement::IsMoving()
{
	return (App->input->GetMouseButtonDown(3) == KEY_DOWN || currentState->playerWalking); //right button or the player is still walking
}

bool PlayerMovement::IsUsingFirstSkill()
{
	return App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondSkill()
{
	return App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdSkill()
{
	return App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthSkill()
{
	return App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFirstItem()
{
	return App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondItem()
{
	return App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdItem()
{
	return App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthItem()
{
	return App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN;
}

void PlayerMovement::CheckStates(PlayerState * previous, PlayerState * current)
{
	if (previous != current)
	{
		previous->ResetTimer();

		previous->Exit();
		current->Enter();

		if (anim != nullptr)
		{
			anim->SendTriggerToStateMachine(current->trigger.c_str());
		}
	}
}
