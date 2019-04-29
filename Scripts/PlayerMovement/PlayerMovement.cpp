#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"


#include "ComponentTransform.h"
#include "GameObject.h"

#include "JSON.h"
#include <assert.h>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"


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
	if (App->input->GetMouseButtonDown(3) == KEY_DOWN) //RIGHT BUTTON
	{
		math::float3 intersectionPoint = math::float3::inf;
		if (App->scene->Intersects(intersectionPoint, "floor"))
		{
			App->navigation->FindPath(gameObject->transform->position, intersectionPoint, path);
		}
	}
	if (path.size() > 0)
	{
		gameObject->transform->SetPosition(path[0]);
		path.erase(path.begin());
	}
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
