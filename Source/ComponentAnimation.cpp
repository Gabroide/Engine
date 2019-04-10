#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleTime.h"

#include "GameObject.h"
#include "Resource.h"
#include "ResourceAnimation.h"
#include "AnimationController.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"

#include "Imgui/include/imgui.h"
#include "JSON.h"
#include "Math/Quat.h"
#include "Math/float3.h"
#include "Brofiler.h"

ComponentAnimation::ComponentAnimation() : Component(nullptr, ComponentType::Animation)
{
	controller = new AnimationController();
	PlayAnimation(100u);
}


ComponentAnimation::~ComponentAnimation()
{
	controller = nullptr;
	anim = nullptr;
	gameobject->isBoneRoot = false;
	RELEASE_ARRAY(animName);
}


void ComponentAnimation::DrawProperties()
{
	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Animation");
		ImGui::PushID("Animation Combo");
		if (ImGui::BeginCombo("", anim != nullptr ? anim->name.c_str() : ""))
		{
			if (guiAnimations.empty())
			{
				guiAnimations = App->resManager->GetAnimationsNamesList(true);
			}
			for (int n = 0; n < guiAnimations.size(); n++)
			{
				bool is_selected = (anim != nullptr ? anim->name == guiAnimations[n] : false);
				if (ImGui::Selectable(guiAnimations[n].c_str(), is_selected) && anim->GetExportedFile() != guiAnimations[n])
				{
					SetAnimation(guiAnimations[n].c_str());
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		else
		{
			guiAnimations.clear();
		}
		ImGui::PopID();
	}
}

void ComponentAnimation::SetAnimation(const char* animationFile)
{
	// Delete previous animation
	if (anim != nullptr)
	{
		App->resManager->DeleteResource(anim->GetUID());
	}

	if (animationFile == nullptr)
	{
		anim = nullptr;
	}
	else
	{
		anim = (ResourceAnimation*)App->resManager->Get(animationFile, TYPE::ANIMATION);
	}
	return;
}

void ComponentAnimation::Update()
{
	PROFILE;
	if (anim != nullptr)
	{
		if (!channelsSetted)
		{
			SetIndexChannels(gameobject);
			channelsSetted = true;
		}

		if (App->time->gameState == GameState::RUN)
		{
			controller->Update(App->time->gameDeltaTime);

			if (gameobject != nullptr)
			{
				UpdateGO(gameobject);
			}
		}
		else if (isPlaying)
		{
			controller->Update(App->time->realDeltaTime);

			if (gameobject != nullptr)
			{
				UpdateGO(gameobject);
			}
		}
	}
}

void ComponentAnimation::UpdateGO(GameObject* go)
{
	PROFILE;
	float3 position;
	Quat rotation;

	if (controller->GetTransform(go->animationIndexChannel, position, rotation))
	{
		go->transform->SetPosition(position);
		go->transform->SetRotation(rotation);
	}

	gameobject->movedFlag = true;

	for (std::list<GameObject*>::iterator it = go->children.begin(); it != go->children.end(); ++it)
	{
		UpdateGO(*it);
	}
}

void ComponentAnimation::PlayAnimation(unsigned blend)
{
	controller->Play(anim, true, blend);
}

Component* ComponentAnimation::Clone() const
{
	return new ComponentAnimation(*this);
}

ComponentAnimation::ComponentAnimation(GameObject * gameobject) : Component(gameobject, ComponentType::Animation)
{
	//anim = new ResourceAnimation();
	controller = new AnimationController();
	PlayAnimation(100u);
}

ComponentAnimation::ComponentAnimation(const ComponentAnimation& component) : Component(component)
{
	anim = (ResourceAnimation*)App->resManager->Get(component.anim->GetUID());
}


bool ComponentAnimation::CleanUp()
{
	if (anim != nullptr)
	{
		App->resManager->DeleteResource(anim->UID);
	}
	return true;
}

void ComponentAnimation::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("animUID", (anim != nullptr) ? anim->GetUID() : 0u);
}

void ComponentAnimation::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("animUID");

	ResourceAnimation* a = (ResourceAnimation*)App->resManager->Get(uid);

	//if (a != nullptr)
	//{
	//	anim = a;
	//}
	//else
	//{
	//	ResourceAnimation* res = (ResourceAnimation*)App->resManager->CreateNewResource(TYPE::ANIMATION, uid);
	//	res->SetExportedFile(std::to_string(uid).c_str());
	//	a = (ResourceAnimation*)App->resManager->Get(uid);
	//	if (a != nullptr)
	//		a = res;
	//}
}

void ComponentAnimation::SetIndexChannels(GameObject* GO)
{
	GO->animationIndexChannel = anim->GetIndexChannel(GO->name.c_str());

	for (const auto& child : GO->children)
	{
		SetIndexChannels(child);
	}

}