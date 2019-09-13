#include "Application.h"
#include "ModuleScene.h"
#include "ModuleTime.h"

#include "GameObject.h"

#include "AOEBossScript.h"

#include "imgui.h"
#include "JSON.h"

AOEBossScript_API Script* CreateScript()
{
	AOEBossScript* instance = new AOEBossScript;
	return instance;
}

void AOEBossScript::Awake()
{
}

void AOEBossScript::Start()
{
	prepParticlesGO = App->scene->FindGameObjectByName("Prep Particles", gameobject);
	if (prepParticlesGO == nullptr)
	{
		LOG("PrepParticles not found");
	}
	beamParticlesGO = App->scene->FindGameObjectByName("Beam Particles", gameobject);
	if (beamParticlesGO == nullptr)
	{
		LOG("beamParticlesGO not found");
	}
	boxTrigger = App->scene->FindGameObjectByName("Hitbox", gameobject);
	if (boxTrigger == nullptr)
	{
		LOG("boxTrigger not found");
	}

	boxTrigger->SetActive(false);
	prepParticlesGO->SetActive(true);
	beamParticlesGO->SetActive(false);
}

void AOEBossScript::Update()
{
	timer += App->time->gameDeltaTime;

	if (timer > timerFade && timer < duration)
	{
		beamParticlesGO->SetActive(true);
		prepParticlesGO->SetActive(false);
		boxTrigger->SetActive(true);
	}
	else if (timer > duration)
	{
		boxTrigger->SetActive(false);
		beamParticlesGO->SetActive(false);
	}
}

void AOEBossScript::Expose(ImGuiContext * context)
{
	ImGui::DragFloat("Time until particles change", &timerFade,0.1f,0.0f,20.0f);
	ImGui::DragFloat("duration", &duration, 0.1f, 1.0f, 20.0f);
}

void AOEBossScript::Serialize(JSON_value * json) const
{
	json->AddFloat("timerFade", timerFade);
	json->AddFloat("duration", duration);
}

void AOEBossScript::DeSerialize(JSON_value * json)
{
	timerFade = json->GetFloat("timerFade", 0.0f);
	duration = json->GetFloat("duration", 0.0f);
}
