#include "Application.h"
#include "MenuSoundsScript.h"
#include "ModuleScene.h"
#include "ComponentButton.h"
#include "ComponentAudioSource.h"

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"

#include "GameObject.h"

MenuSoundsScript_API Script* CreateScript()
{
	MenuSoundsScript* instance = new MenuSoundsScript;
	return instance;
}

void MenuSoundsScript::Start()
{

	Menu = App->scene->FindGameObjectByName("Menu");
	buttons = Menu->GetComponentsInChildren(ComponentType::Button);

	audioHovered = (ComponentAudioSource*)App->scene->FindGameObjectByName("HoveredAudio")->GetComponentOld(ComponentType::AudioSource);
	assert(audioHovered != nullptr);

	audioClick = (ComponentAudioSource*)App->scene->FindGameObjectByName("ClickAudio")->GetComponentOld(ComponentType::AudioSource);
	assert(audioClick != nullptr);

}

void MenuSoundsScript::Update()
{
	bool nothingHovered = true;
	bool nothingClicked = true;

	// Check if button clicked or hovered
	for (auto button : buttons)
	{

		if (((Button*)button)->IsHovered())
		{
			if (!itemHovered) audioHovered->Play();
			itemHovered = true;
			nothingHovered = false;
		}
		if (((Button*)button)->IsPressed())
		{
			if (!itemClicked) audioClick->Play();
			itemClicked = true;
			nothingClicked = false;
		}
	}

	itemHovered = !nothingHovered;
	itemClicked = !nothingClicked;
}
