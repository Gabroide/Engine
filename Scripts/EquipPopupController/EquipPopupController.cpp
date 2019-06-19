#include "EquipPopupController.h"

#include "Application.h"
#include "ModuleScene.h"
#include "ModuleInput.h"

#include "GameObject.h"
#include "ComponentButton.h"
#include "ComponentText.h"
#include "ComponentImage.h"

#include "InventoryScript.h"
#include "SkillTreeController.h"
#include "Item.h"
#include "Skill.h"

EquipPopupController_API Script* CreateScript()
{
	EquipPopupController* instance = new EquipPopupController;
	return instance;
}

void EquipPopupController::Start()
{
	inventory = App->scene->FindGameObjectByName("Inventory")->GetComponent<InventoryScript>();
	assert(inventory != nullptr);

	skillTree = App->scene->FindGameObjectByName("Skills")->GetComponent<SkillTreeController>();
	assert(inventory != nullptr);

	popupGO = App->scene->FindGameObjectByName("PopUpBackground");
	assert(popupGO != nullptr);

	background = popupGO->GetComponent<ComponentImage>();
	assert(background != nullptr);

	items = App->scene->FindGameObjectByName("PopUpItems", popupGO)->GetComponent<Button>();
	assert(items != nullptr);

	skills = App->scene->FindGameObjectByName("PopUpSkills", popupGO)->GetComponent<Button>();
	assert(skills != nullptr);

	unequip = App->scene->FindGameObjectByName("PopUpUnequip", popupGO)->GetComponent<Button>();
	assert(unequip != nullptr);

	title = App->scene->FindGameObjectByName("PopUpEquipTitle", popupGO)->GetComponent<Text>();
	assert(title != nullptr);

	GameObject* HUD = App->scene->FindGameObjectByName("GameHUB");
	assert(HUD != nullptr);
	hudButtons.emplace_back(App->scene->FindGameObjectByName("One", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Two", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Three", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("For", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("Q", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("W", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("E", HUD)->GetComponent<Button>());
	hudButtons.emplace_back(App->scene->FindGameObjectByName("R", HUD)->GetComponent<Button>());

	std::list<GameObject*> list = App->scene->FindGameObjectByName("PopUpSlots", popupGO)->children;

	slots = { std::begin(list), std::end(list) };
}

void EquipPopupController::Update()
{
	//Check if has to close PopUp
	if (popupGO->isActive() && !background->isHovered && !items->IsHovered() && !skills->IsHovered() && (App->input->GetMouseButtonDown(1) == KEY_DOWN || App->input->GetMouseButtonDown(3) == KEY_DOWN))
	{
		popupGO->SetActive(false);
		return;
	}

	//Check if has to open PopUp
	for (int i = 0; i < 8; ++i)
	{
		if (hudButtons[i]->IsHovered() && App->input->GetMouseButtonDown(3) == KEY_DOWN) {
			activeButton = hudButtons[i];
			if (!popupGO->isActive())
			{
				FillSkillSlots();
				FillItemSlots();
				popupGO->SetActive(true);
			}
			break;
		}
	}

	if (!popupGO->isActive()) { return; }

	//Update Popup info
	ChangePopUpSlots();


	//Check if selected
	for (int i = 0; i < 16; ++i)
	{
		if (!slots[i]->isActive())
		{
			break;
		}
		if (slots[i]->GetComponent<ComponentImage>()->isHovered)
		{
			//TODO: Hacer Hover
			//TODO: Show info
			if (App->input->GetMouseButtonDown(1) == KEY_DOWN)
			{
				Assign(i);
			}
		}
	}

}

void EquipPopupController::Assign(int i)
{
	if (skillsShowing)
	{
		activeButton->UpdateImageByResource(skillsList[i].spriteActive);
		activeButton->gameobject->children.front()->GetComponent<ComponentImage>()->texture = skillsList[i].spriteActive;
		//TODO: Pass info to player
	}
	else
	{
		activeButton->UpdateImageByName(itemsList[i].sprite);		
		activeButton->gameobject->children.front()->GetComponent<ComponentImage>()->UpdateTexture(itemsList[i].sprite);
		//TODO: Pass info to player
	}
	popupGO->SetActive(false);
}

void EquipPopupController::ChangePopUpSlots()
{
	if (items->IsPressed() && skillsShowing)
	{
		skillsShowing = false;
		title->text = "Items";
		FillItemSlots();
	}

	if (skills->IsPressed() && !skillsShowing)
	{
		skillsShowing = true;
		title->text = "Skills";
		FillSkillSlots();
	}
}


void EquipPopupController::FillLists()
{
	
	skillsList.clear();
	itemsList.clear();
	skillsList = skillTree->GetActiveSkills();
	itemsList = inventory->GetQuickItems();
	
	if (skillsShowing) 
	{
		FillSkillSlots();
	}
	else
	{
		FillItemSlots();
	}
}

void EquipPopupController::FillSkillSlots()
{
	
	for (int i = 0; i < 16; ++i)
	{
		slots[i]->SetActive(false);
		if (i < skillsList.size())
		{
			slots[i]->GetComponent<ComponentImage>()->texture = skillsList[i].spriteActive;
			slots[i]->SetActive(true);
		}
	}
}

void EquipPopupController::FillItemSlots()
{
	
	if (skillsShowing) {
		return;
	}

	for (int i = 0; i < 16; ++i)
	{
		slots[i]->SetActive(false);

		if (i < itemsList.size())
		{
			slots[i]->GetComponent<ComponentImage>()->UpdateTexture(itemsList[i].sprite);
			slots[i]->SetActive(true);
		}

	}
}
