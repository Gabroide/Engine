#include "BaseScript.h"

#include "Application.h"
#include "GameObject.h"
#include "imgui.h"
#include "JSON.h"

#include "ModuleScript.h"

Script::Script(const Script & script) : Component(script)
{
	name = script.name;
	gameObject = script.gameObject;
	App = script.App;
}

Script::Script(GameObject * gameobject = nullptr) : Component(gameObject, ComponentType::Script)
{
	gameObject = gameObject;
}

Script::Script() : Component(nullptr, ComponentType::Script)
{
}


Script::~Script()
{
	App->scripting->RemoveScript(this, name);
}

Script * Script::Clone() const
{
	return new Script(*this);
}

void Script::SetGameObject(GameObject* go)
{
	gameObject = go;
}

void Script::DrawProperties()
{
	ImGui::PushID(this);
	if (ImGui::CollapsingHeader((name + "(Script)").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool removed = Component::DrawComponentState();
		if (removed)
		{
			return;
		}

		ImGuiContext * context = ImGui::GetCurrentContext();
		Expose(context); //TODO: remove expose
	}
	ImGui::PopID();
}

void Script::SetApp(Application* app)
{
	this->App = app;
}

void Script::Load(JSON_value * value) 
{
	Component::Load(value);
	const char* retrievedName = value->GetString("script");
	JSON_value* scriptInfo = value->GetValue("scriptInfo");
	if (scriptInfo != nullptr)
	{
		DeSerialize(scriptInfo);
	}
}

void Script::Save(JSON_value * value) const
{
	Component::Save(value);
	value->AddString("script", name.c_str());
	JSON_value *scriptInfo = value->CreateValue();
	Serialize(scriptInfo);
	value->AddValue("scriptInfo", *scriptInfo);
}

void Script::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
}