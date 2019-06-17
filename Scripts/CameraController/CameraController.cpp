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
	assert(player != nullptr);
	offset = gameobject->transform->GetPosition() - player->transform->GetPosition();
}

void CameraController::Update()
{
	if (isShaking)
	{
		float x = rand.Float();
	}
	else
	{
		math::float3 newPosition = offset + player->transform->GetPosition();
		gameobject->transform->SetPosition(newPosition);
	}
}

void CameraController::Shake(float duration, float intensity)
{
	isShaking = true;
	shakeDuration = duration;
	shakeIntensity = intensity;
	return;
}
