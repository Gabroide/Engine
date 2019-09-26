#include "LootDropScript.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ResourcePrefab.h"

#include "GameObject.h"
#include "ComponentTransform.h"

#include "imgui.h"
#include "JSON.h"

LootDropScript_API Script* CreateScript()
{
	LootDropScript* instance = new LootDropScript;
	return instance;
}

void LootDropScript::Start()
{
	itemListGO = App->scene->FindGameObjectByName("ItemsList");
	if (itemListGO == nullptr)
		LOG("Warning: ItemList GO couldn't be found, items will spawn with root as a parent.");
}

void LootDropScript::Expose(ImGuiContext* context)
{
	//ImGui::InputInt("Num Items", &numberOfItems);


	/*for (int i = 0; i < itemList.size(); ++i)
	{
		ImGui::Text("Item: ");
		ImGui::SameLine();
		ImGui::Text(itemList[i]->name.c_str());
		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();
		ImGui::Text("Drop:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(itemList[i]).c_str());

	}*/

	ImGui::Text("Item: ");
	char* itemName = new char[64];
	strcpy_s(itemName, strlen(itemPrefabName.c_str()) + 1, itemPrefabName.c_str());
	ImGui::InputText("Prefab Name", itemName, 64);
	itemPrefabName = itemName;
	delete[] itemName;
}

void LootDropScript::Serialize(JSON_value* json) const
{
	json->AddString("ItemPrefabName", itemPrefabName.c_str());
}

void LootDropScript::DeSerialize(JSON_value* json)
{
	itemPrefabName = json->GetString("ItemPrefabName", "");
}

void LootDropScript::DropItems()
{
	if (itemListGO != nullptr)
	{
		GameObject* spawnedItem = App->scene->Spawn(itemPrefabName.c_str(), gameobject->transform->position, itemListGO->transform->rotation, itemListGO);
		spawnedItem->transform->Rotate(rotationOffset);
	}
	else
		App->scene->Spawn(itemPrefabName.c_str());
}
