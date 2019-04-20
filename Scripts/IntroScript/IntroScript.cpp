#include "IntroScript.h"

#include "Application.h"
#include "ModuleTime.h"

#include "ComponentTransform.h"
#include "GameObject.h"

#include "JSON.h"
#include "imgui.h"

IntroScript_API Script* CreateScript()
{
	IntroScript* instance = new IntroScript;
	return instance;
}

void IntroScript::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::DragFloat("Speed", &speed, 1.0f, 0.0f, 100.0f);
	ImGui::DragFloat3("Final position", (float*)&finalPosition, 1.0f, -10000.0f, 10000.0f);
}

void IntroScript::Start()
{
	initialPosition = cameraPosition = gameObject->transform->GetPosition();
	distanceNormalized = (finalPosition - cameraPosition).Normalized();
}

void IntroScript::Update()
{
	if (!introDone)
	{
		cameraPosition += (distanceNormalized * speed * App->time->gameDeltaTime);
		gameObject->transform->SetPosition(cameraPosition);

		if (fabsf(cameraPosition.x) >= fabsf(finalPosition.x))
			introDone = true;
	}
}

void IntroScript::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("speed", speed);
	json->AddFloat3("finalPosition", finalPosition);
}

void IntroScript::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	speed = json->GetFloat("speed");
	finalPosition = json->GetFloat3("finalPosition");
}
