#include "EnemyLoot.h"
#include "Application.h"
#include "Resource.h"
#include "ModuleScene.h"
#include "ModuleScript.h"
#include "ModuleResourceManager.h"

#include "ComponentTransform.h"

#include "GameObject.h"

#include "ItemPicker.h"

#include "Math/float4x4.h"


#pragma warning(disable : 4996)

EnemyLoot_API Script* CreateScript()
{
	EnemyLoot* instance = new EnemyLoot;
	return instance;
}

void EnemyLoot::Start()
{
	std::list<GameObject*> GOs = gameobject->children;

	for (std::list<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); ++it)
	{
		if ((*it)->GetComponent<ItemPicker>() != nullptr)
		{
			items.push_back(std::make_pair((*it), 100));
		}
	}
}

void EnemyLoot::Update()
{
}

void EnemyLoot::GenerateLoot()
{
	for (int i = 0; i < items.size(); ++i)
	{
		go = items[i].first;
		if ((rand() % 100) < items[i].second)
		{
			go->SetActive(true);
		}
		GameObject* goAux = App->scene->FindGameObjectByName(itemName.c_str(), gameobject);
		go = new GameObject(*goAux);
		gameobject->RemoveChild(goAux);
		gameobject->InsertChild(go);
		math::float3 pos = gameobject->transform->GetPosition();
		float x = static_cast <float>(rand() / static_cast <float> (RAND_MAX / 200));
		float z = static_cast <float>(rand() / static_cast <float> (RAND_MAX / 200));
		pos.x += x;
		pos.y += 30;
		pos.z += z;
		go->transform->SetPosition(pos);
	}
	items.clear();
}


void EnemyLoot::Expose(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ImGui::Separator();
	ImGui::Text("List of items to drop:");
	for (int i = 0; i < items.size(); ++i)
	{
		ImGui::Text("Item: ");
		ImGui::SameLine();
		ImGui::Text(items[i].first->name.c_str());
		ImGui::SameLine();
		ImGui::Separator();
		ImGui::SameLine();
		ImGui::Text("Drop:");
		ImGui::SameLine();
		ImGui::Text(std::to_string(items[i].second).c_str());

	}
	if (items.size() == 0)
	{
		ImGui::Text("No items!");
	}

	GetItems();
	if (itemList.size() > 0 && ImGui::BeginCombo("Item name", itemName.c_str()))
	{
		for (int i = 0; i < itemList.size(); i++)
		{
			if (ImGui::Selectable(itemList[i].c_str()))
			{
				itemName = itemList[i];
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragInt("Drop %", &drop, 0.01f, 0, 100);

	if(ImGui::Button("Add item"))
	{
		GameObject* goAux = App->scene->FindGameObjectByName(itemName.c_str());
		go = new GameObject(*goAux);
		App->scene->root->RemoveChild(goAux);
		gameobject->InsertChild(go);
		items.push_back(std::make_pair(go, drop));
		if (go->isActive())
		{
			go->SetActive(false);
		}
		goName = "GO Name";
		itemName = "";
		uid = 0;
		drop = 0;
	}
}


void EnemyLoot::GetItems()
{
	itemList.clear();
	std::vector<std::string> items;
	std::list<GameObject*> GOs = App->scene->root->children;

	for (std::list<GameObject*>::iterator it = GOs.begin(); it != GOs.end(); ++it)
	{
		if ((*it)->GetComponent<ItemPicker>() != nullptr)
		{
			itemList.push_back(((*it)->name).c_str());
		}
	}


	return;
}