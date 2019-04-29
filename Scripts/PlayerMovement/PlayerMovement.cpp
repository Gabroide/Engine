#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"

#include "ComponentTransform.h"
#include "GameObject.h"

#include "JSON.h"
#include <assert.h>
#include "imgui.h"
#include <iostream>
#include "Globals.h"

PlayerMovement_API Script* CreateScript()
{
	PlayerMovement* instance = new PlayerMovement;
	return instance;
}

void PlayerMovement::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::InputFloat("speed", &speed);
}

void PlayerMovement::Start()
{
	LOG("Started player movement script");
}
void PlayerMovement::Update()
{
	currentPosition = gameObject->transform->GetPosition();
	currentRotation = gameObject->transform->GetRotation();

	rotation = Quat(0, 0, 0, 1);

	if(App->input->IsKeyPressed(SDL_SCANCODE_A))
	{
		rotation = Quat(float3::unitY, -speedAngular* App->time->gameDeltaTime);
	}
	if (App->input->IsKeyPressed(SDL_SCANCODE_D))
	{
		rotation = Quat(float3::unitY, speedAngular* App->time->gameDeltaTime);
	}
	if (App->input->IsKeyPressed(SDL_SCANCODE_S))
	{
		currentPosition -= gameObject->transform->front * speed * App->time->gameDeltaTime;
	}
	if (App->input->IsKeyPressed(SDL_SCANCODE_W))
	{
		currentPosition += gameObject->transform->front * speed * App->time->gameDeltaTime;
		
	}

	//set the rotation
	gameObject->transform->SetRotation(rotation.Mul(currentRotation));
	gameObject->transform->SetPosition(currentPosition);
}

void PlayerMovement::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("speed", speed);
}

void PlayerMovement::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	speed = json->GetFloat("speed");
}
