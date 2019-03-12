#include "Application.h"

#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"

#include "GameObject.h"
#include "Animation.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"

#include "Imgui/include/imgui.h"
#include "JSON.h"
#include "Math/Quat.h"
#include "Math/float3.h"

void ComponentAnimation::DrawProperties()
{
	//We need to have here:

	//Name of the animation
	ImGui::Text(anim->animationName.c_str());

	//Number of frames the animation has
	ImGui::Text("%f frames", anim->numberFrames);

	//Duration of the animation
	ImGui::Text("%i seconds", anim->durationInSeconds);

	//Switch between animation frames
	if (ImGui::InputInt("Frame #", &anim->currentSample))
	{
		if (anim->currentSample < 0)
		{
			anim->currentSample = anim->numberFrames - 1;
		}
		else if (anim->currentSample >= anim->numberFrames)
		{
			anim->currentSample = 0;
		}
	}

	//Play
	ImGui::ArrowButton("Play", ImGuiDir_Right);

}

void ComponentAnimation::Update(Frame* frame, Channel* channel)
{

}

Component* ComponentAnimation::Clone() const
{
	return new ComponentAnimation(*this);
}

ComponentAnimation::ComponentAnimation(GameObject * gameobject) : Component(gameobject, ComponentType::Animation)
{
	anim = new Animation();
}

ComponentAnimation::ComponentAnimation(const ComponentAnimation& component) : Component(component)
{
	anim = component.anim;

	App->resManager->AddAnim(anim);
}


ComponentAnimation::ComponentAnimation() : Component(nullptr, ComponentType::Animation)
{
}


ComponentAnimation::~ComponentAnimation()
{
	anim = nullptr;
}

bool ComponentAnimation::CleanUp()
{
	if (anim != nullptr)
	{
		App->resManager->DeleteAnim(anim->UID);
	}
	return true;
}

void ComponentAnimation::Save(JSON_value* value) const
{
	Component::Save(value);
	value->AddUint("animUID", anim->UID);
}

void ComponentAnimation::Load(JSON_value* value)
{
	Component::Load(value);
	unsigned uid = value->GetUint("animUID");

	Animation* a = App->resManager->GetAnim(uid);

	if (a != nullptr)
	{
		anim = a;

	}
	else
	{
		char* data = nullptr;
		App->fsystem->Load((ANIMATIONS + std::to_string(uid) + ANIMATIONEXTENSION).c_str(), &data);
		anim->Load(data, uid);
		App->resManager->AddAnim(anim);
	}
}

Frame* ComponentAnimation::InterpolateFrame(const Frame* first, const Frame* second, float lambda) const
{
	Frame* newFrame;
	//for (unsigned i = 0u; i < anim->numberOfChannels; i++)
	//{
	//	math::float4x4 newTransform = InterpolateFloat4x4(first->channels[i]->channelTransform, second->channels[i]->channelTransform, lambda);
	//	newFrame->channels[i]->channelName = first->channels[i]->channelName;
	//	newFrame->channels[i]->channelTransform = newTransform;
	//}

	return newFrame;
}

math::float4x4 ComponentAnimation::InterpolateFloat4x4(const math::float4x4& first, const math::float4x4& second, float lambda) const
{
	math::float3 firstPosition;
	math::Quat firstRotation;
	math::float3 firstScale;
	first.Decompose(firstPosition, firstRotation, firstScale);

	math::float3 secondPosition;
	math::Quat secondRotation;
	math::float3 secondScale;
	second.Decompose(secondPosition, secondRotation, secondScale);

	math::float3 newPosition = InterpolateFloat3(firstPosition, secondPosition, lambda);
	math::Quat newRotation = InterpolateQuat(firstRotation, secondRotation, lambda);

	math::float4x4 newMatrix = math::float4x4::FromTRS(newPosition, newRotation, math::float3::one);

	return newMatrix;
}

math::float3 ComponentAnimation::InterpolateFloat3(const math::float3& first, const math::float3& second, float lambda) const
{
	return first * (1.0f - lambda) + second * lambda;
}

math::Quat ComponentAnimation::InterpolateQuat(const math::Quat& first, const math::Quat& second, float lambda) const
{
	Quat result;
	float dot = first.Dot(second);

	if (dot >= 0.0f) // Interpolate through the shortest path
	{
		result.x = first.x*(1.0f - lambda) + second.x*lambda;
		result.y = first.y*(1.0f - lambda) + second.y*lambda;
		result.z = first.z*(1.0f - lambda) + second.z*lambda;
		result.w = first.w*(1.0f - lambda) + second.w*lambda;
	}
	else
	{
		result.x = first.x*(1.0f - lambda) - second.x*lambda;
		result.y = first.y*(1.0f - lambda) - second.y*lambda;
		result.z = first.z*(1.0f - lambda) - second.z*lambda;
		result.w = first.w*(1.0f - lambda) - second.w*lambda;
	}

	result.Normalize();

	return result;
}

void ComponentAnimation::OffsetChannels(GameObject* GO)
{
	//for (const auto& currentChannel : anim->channels)
	//{

	math::float3 positionOffset = math::float3::zero;
	math::Quat rotationOffset = math::Quat::identity;

	unsigned index = anim->GetIndexChannel(GO->name.c_str());

	if (index != 999u)
	{
		positionOffset = anim->channels[index]->positionSamples[0] - GO->transform->GetPosition();
		rotationOffset = anim->channels[index]->rotationSamples[0].Inverted()*GO->transform->GetRotation();
		for (unsigned i = 0u; i < anim->channels[index]->numPositionKeys; i++)
		{
			anim->channels[index]->positionSamples[i] += positionOffset;
		}
		for (unsigned j = 0u; j < anim->channels[index]->numRotationKeys; j++)
		{
			anim->channels[index]->rotationSamples[j] = anim->channels[index]->rotationSamples[j] * rotationOffset;
		}
	}

	for (const auto& child : GO->children)
	{
		OffsetChannels(child);
	}

	//}
}