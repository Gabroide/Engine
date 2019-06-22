#include "CameraController.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "imgui.h"
#include "JSON.h"

CameraController_API Script* CreateScript()
{
	CameraController* instance = new CameraController;
	return instance;
}

void CameraController::Start()
{

	player = App->scene->FindGameObjectByName("Player");
	//assert(player != nullptr);
	if (player != nullptr)
	{
		offset = gameobject->transform->GetPosition() - player->transform->GetPosition();
	}
	else
	{
		LOG("Warning: player GO couldn't be found.");
	}
}

void CameraController::Update()
{
	if (player != nullptr)
	{
		math::float3 newPosition = offset + player->transform->GetPosition();
		gameobject->transform->SetPosition(newPosition);
	}
}
