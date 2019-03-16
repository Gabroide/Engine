#include "IntroScript.h"

#include "ComponentTransform.h"
#include "GameObject.h"

#include "imgui.h"

IntroScript_API IntroScript* CreateScript()
{
	IntroScript* instance = new IntroScript;
	return instance;
}

void IntroScript::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::DragFloat("Speed", &speed, 1.0f, 0.0f, 100.0f);
	ImGui::DragFloat3("Final position", (float*)&finalPosition, 1.0f, -1000.0f, 1000.0f);
}

void IntroScript::Start()
{
	cameraPosition = gameObject->transform->GetPosition();

	distanceToMove = finalPosition.z - cameraPosition.z;

	if (finalPosition.z < cameraPosition.z)
	{
		speed *= -1;
		distanceToMove = cameraPosition.z - finalPosition.z;
	}
}

void IntroScript::Update()
{
	if (!introDone)
	{
		cameraPosition.z += speed;
		gameObject->transform->SetPosition(cameraPosition);
		distanceMoved += fabsf(speed);

		if (distanceMoved >= distanceToMove)
			introDone = true;
	}
}
