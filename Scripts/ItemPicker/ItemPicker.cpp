#include "ItemPicker.h"

ItemPicker_API Script* CreateScript()
{
	ItemPicker* instance = new ItemPicker;
	return instance;
}

void ItemPicker::Expose(ImGuiContext* context)
{
	//Expose Item attributes
}


void ItemPicker::Start()
{
	item.name = this->name;
	item.description = this->description;
	item.sprite = this->sprite;
	item.type = this->type;
}

void ItemPicker::Update()
{
	//Check bounding box for collider
}