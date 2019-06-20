#include "MeleeSkill.h"

#include "ComponentBoxTrigger.h"

#include "PlayerMovement.h"

MeleeSkill::MeleeSkill(PlayerMovement* PM, const char* trigger, ComponentBoxTrigger* attackBox) :
	BasicSkill(PM, trigger), attackBoxTrigger(attackBox)
{
}

MeleeSkill::~MeleeSkill()
{
}

void MeleeSkill::Expose(ImGuiContext* context)
{
	BasicSkill::Expose(context);
}

void MeleeSkill::Serialize(JSON_value* json) const
{
	BasicSkill::Serialize(json);
}

void MeleeSkill::DeSerialize(JSON_value* json)
{
	BasicSkill::DeSerialize(json);
}

void MeleeSkill::Reset()
{
	BasicSkill::Reset();

	if (attackBoxTrigger != nullptr)
	{
		attackBoxTrigger->Enable(false);
	}
}