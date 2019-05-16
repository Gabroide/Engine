#include "ResourceScene.h"

#include "Globals.h"
#include "Application.h"
#include "ModuleFileSystem.h"
#include "ModuleScene.h"
#include "ModuleSpacePartitioning.h"
#include "ModuleNavigation.h"

#include "GameObject.h"
#include "ComponentRenderer.h"

#include "JSON.h"

ResourceScene::ResourceScene(unsigned uid) : Resource(uid, TYPE::SCENE)
{
}

ResourceScene::ResourceScene(const ResourceScene& resource) : Resource(resource)
{
}

ResourceScene::~ResourceScene()
{
}

void ResourceScene::SaveMetafile(const char* file) const
{
	JSON* json = new JSON();
	JSON_value* meta = json->CreateValue();
	struct stat statFile;
	stat(file, &statFile);

	json->AddValue("Scene", *meta);

	std::string filepath(file);
	filepath += METAEXT;
	App->fsystem->Save(filepath.c_str(), json->ToString().c_str(), json->Size());
}

void ResourceScene::Save(const GameObject& rootGO)
{
	JSON *json = new JSON();
	JSON_value *array = json->CreateValue(rapidjson::kArrayType);
	rootGO.Save(array);
	json->AddValue("GameObjects", *array);

	App->fsystem->Save(file.c_str(), json->ToString().c_str(), json->Size());
	RELEASE(json);
}

bool ResourceScene::Load()
{
	char* data = nullptr;

	if (App->fsystem->Load(exportedFile.c_str(), &data) == 0)
	{
		RELEASE_ARRAY(data);
		return false;
	}

	JSON *json = new JSON(data);
	JSON_value* gameobjectsJSON = json->GetValue("GameObjects");
	std::map<unsigned, GameObject*> gameobjectsMap; //Necessary to assign parent-child efficiently
	gameobjectsMap.insert(std::pair<unsigned, GameObject*>(App->scene->canvas->UUID, App->scene->canvas));

	std::list<ComponentRenderer*> renderers;

	for (unsigned i = 0; i < gameobjectsJSON->Size(); i++)
	{
		JSON_value* gameobjectJSON = gameobjectsJSON->GetValue(i);
		GameObject *gameobject = new GameObject();
		gameobject->Load(gameobjectJSON);
		if (gameobject->UUID != 1)
		{
			gameobjectsMap.insert(std::pair<unsigned, GameObject*>(gameobject->UUID, gameobject));

			std::map<unsigned, GameObject*>::iterator it = gameobjectsMap.find(gameobject->parentUUID);
			if (it != gameobjectsMap.end())
			{
				gameobject->parent = it->second;
				gameobject->parent->children.push_back(gameobject);
			}
			else if (gameobject->parentUUID == 0)
			{
				gameobject->parent = App->scene->root;
				gameobject->parent->children.push_back(gameobject);
			}

			ComponentRenderer* renderer = nullptr;
			renderer = (ComponentRenderer*)gameobject->GetComponentOld(ComponentType::Renderer);
			if (renderer != nullptr)
			{
				renderers.push_back(renderer);
			}
		}
	}

	if (!App->scene->isCleared)
	{
		//Recursive UID reassign
		AssignNewUUID(App->scene->root, 0u);
	}

	//Link Bones after all the hierarchy is imported
	for (ComponentRenderer* cr : renderers)
	{
		if (cr->mesh != nullptr)
		{
			cr->LinkBones();
		}
	}

	App->navigation->sceneLoaded(json);

	RELEASE_ARRAY(data);
	RELEASE(json);

	return true;
}

void ResourceScene::AssignNewUUID(GameObject* go, unsigned parentUID)
{

	if (go != App->scene->root && go != App->scene->canvas)
	{
		go->parentUUID = parentUID;
		go->UUID = App->scene->GetNewUID();
	}

	for (auto& child : go->children)
	{
		AssignNewUUID(child, go->UUID);
	}
}