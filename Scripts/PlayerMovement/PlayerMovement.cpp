#include "PlayerMovement.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleTime.h"
#include "ModuleScene.h"
#include "ModuleNavigation.h"
#include "PlayerState.h"
#include "PlayerStateFirstAttack.h"
#include "PlayerStateSecondAttack.h"
#include "PlayerStateThirdAttack.h"
#include "PlayerStateDash.h"
#include "PlayerStateIdle.h"
#include "PlayerStateWalk.h"
#include "PlayerStateDeath.h"
#include "PlayerStateUppercut.h"
#include "EnemyControllerScript.h"

#include "ComponentAnimation.h"
#include "ComponentBoxTrigger.h"
#include "ComponentTransform.h"
#include "ComponentImage.h"
#include "GameObject.h"

#include "DamageController.h"
#include "DamageFeedbackUI.h"
#include "EnemyControllerScript.h"

#include "JSON.h"
#include <assert.h>
#include <string>
#include "imgui.h"
#include "Globals.h"
#include "debugdraw.h"

PlayerMovement_API Script* CreateScript()
{
	PlayerMovement* instance = new PlayerMovement;
	return instance;
}

PlayerMovement::PlayerMovement()
{
	// Register Skills
	allSkills[SkillType::NONE] = new PlayerSkill();
	allSkills[SkillType::DASH] = new PlayerSkill(SkillType::DASH);
	allSkills[SkillType::UPPERCUT] = new PlayerSkill(SkillType::UPPERCUT);

	// Default ability keyboard allocation
	activeSkills[HUB_BUTTON_Q] = SkillType::DASH;
	activeSkills[HUB_BUTTON_W] = SkillType::UPPERCUT;
}

PlayerMovement::~PlayerMovement()
{
	for (auto it = allSkills.begin(); it != allSkills.end(); ++it) delete it->second;
	allSkills.clear();
}

void PlayerMovement::Expose(ImGuiContext* context)
{
	if (ImGui::Checkbox("Show Item Cooldown", &showItemCooldowns)) ActivateHudCooldownMask(showItemCooldowns, HUB_BUTTON_1, HUB_BUTTON_4);

	ImGui::SetCurrentContext(context);

	//Exposing durations this should access to every class instead of allocating them in PlayerMovement, but for now scripts don't allow it
	ImGui::DragFloat("Walking speed", &walkingSpeed, 0.01f, 10.f, 500.0f);
	ImGui::DragFloat("Out of NavMesh pos correction XZ", &OutOfMeshCorrectionXZ, 1.f, 0.f, 1000.f);
	ImGui::DragFloat("Out of NavMesh pos correction Y", &OutOfMeshCorrectionY, 1.f, 0.f, 500.f);
	ImGui::DragFloat("Max walking distance", &maxWalkingDistance, 100.f, 0.f, 100000.0f);

	ImGui::Spacing();
	float maxHP = stats.health;
	float maxMP = stats.mana;
	stats.Expose("Player Stats");
	if (maxHP != stats.health)
	{
		health += stats.health - maxHP;
		if (health > stats.health) health = stats.health;
		else if (health < 0) health = 0;
	}
	if (maxMP != stats.mana)
	{
		mana += stats.mana - maxMP;
		if (mana > stats.mana) mana = stats.mana;
		else if (mana < 0) mana = 0;
	}

	ImGui::Spacing();
	ImGui::Text("HP/MP Regen Timers");
	ImGui::DragFloat("Out of Combat time", &outCombatMaxTime, 1.f, 0.f, 10.f);
	ImGui::DragFloat("After skill time (MP)", &manaRegenMaxTime, 1.f, 0.f, 10.f);

	ImGui::Spacing();
	ImGui::Text("Cooldowns");
	if (ImGui::Checkbox("Show Ability Cooldown", &showAbilityCooldowns)) ActivateHudCooldownMask(showAbilityCooldowns, HUB_BUTTON_Q, HUB_BUTTON_R);
	ImGui::DragFloat("General Ability Cooldown", &hubGeneralAbilityCooldown, 1.0F, 0.0F, 10.0F);

	ImGui::Spacing();
	ImGui::Text("Skills");
	for (auto it = allSkills.begin(); it != allSkills.end(); ++it)
	{
		switch (it->first)
		{
		case SkillType::DASH:
			it->second->Expose("Dash");
			break;
		case SkillType::UPPERCUT:
			it->second->Expose("Uppercut");
			break;
		case SkillType::NONE:
		default:
			break;
		}
	}

	ImGui::Spacing();
	// Stats Debug
	ImGui::Text("Play Stats Debug");
	ImGui::Text("HP: %f / %f", health, stats.health);
	ImGui::Text("MP: %f / %f", mana, stats.mana);
	ImGui::Text("Strength: %i", stats.strength);
	ImGui::Text("Dexterity: %i", stats.dexterity);
	ImGui::Text("HP Regen: %f pts/s", stats.hpRegen);
	ImGui::Text("Dexterity: %f pts/s", stats.manaRegen);
}

void PlayerMovement::CreatePlayerStates()
{
	playerStates.reserve(8);

	playerStates.push_back(walk = new PlayerStateWalk(this, "Walk"));
	if (dustParticles == nullptr)
	{
		LOG("Dust Particles not found");
	}
	else
	{
		LOG("Dust Particles found");
		dustParticles->SetActive(false);
		walk->dustParticles = dustParticles;
	}
	playerStates.push_back(firstAttack = new PlayerStateFirstAttack(this, "FirstAttack", 
		math::float3(150.f, 100.f, 100.f), 0.7f, 0.9f));
	playerStates.push_back(secondAttack = new PlayerStateSecondAttack(this, "SecondAttack",
		math::float3(150.f, 100.f, 100.f), 0.6f, 0.8f));
	playerStates.push_back(thirdAttack = new PlayerStateThirdAttack(this, "ThirdAttack",
		math::float3(100.f, 200.f, 100.f), 0.75f, 0.9f));
	playerStates.push_back(idle = new PlayerStateIdle(this, "Idle"));
	playerStates.push_back(death = new PlayerStateDeath(this, "Death"));
	playerStates.push_back(uppercut = new PlayerStateUppercut(this, "Uppercut", math::float3(100.f, 200.f, 100.f)));
	playerStates.push_back(dash = new PlayerStateDash(this, "Dash", math::float3(80.f, 100.f, 200.f)));

	allSkills[SkillType::DASH]->state = dash;
	allSkills[SkillType::UPPERCUT]->state = uppercut;
}

void PlayerMovement::Start()
{
	dustParticles = App->scene->FindGameObjectByName("WalkingDust");
	dashFX = App->scene->FindGameObjectByName("DashFX");
	dashMesh = App->scene->FindGameObjectByName("DashMesh");

	GameObject* damageGO = App->scene->FindGameObjectByName("Damage");
	if (damageGO == nullptr)
	{
		LOG("Damage controller GO couldn't be found \n");
	}
	else
	{
		damageController = damageGO->GetComponent<DamageController>();
		if (damageController == nullptr)
		{
			LOG("Damage controller couldn't be found \n");
		}
	}

	GameObject* damageFeedback = App->scene->FindGameObjectByName("RedScreen");
	if (damageFeedback == nullptr)
	{
		LOG("Damage Feedback blood couldn't be found \n");
	}
	else
	{
		damageUIFeedback = damageFeedback->GetComponent<DamageFeedbackUI>();
		if (damageUIFeedback == nullptr)
		{
			LOG("Damage UI feedback script couldn't be found \n");
		}
	}

	CreatePlayerStates();
	if (dashFX == nullptr)
	{
		LOG("DashFX Gameobject not found");
	}
	else
	{
		LOG("DashFX found");
		dashFX->SetActive(false);
		dash->dashFX = dashFX;
	}

	if (dashMesh == nullptr)
	{
		LOG("DashMesh Gameobject not found");
	}
	else
	{
		LOG("DashMesh found");
		dashMesh->SetActive(false);
		dash->meshOriginalScale = dashMesh->transform->scale;
		dash->dashMesh = dashMesh;
	}

	currentState = idle;

	anim = gameobject->GetComponent<ComponentAnimation>();
	if (anim == nullptr)
	{
		LOG("The GameObject %s has no Animation component attached \n", gameobject->name);
	}

	GameObject* hitBoxAttackGameObject = App->scene->FindGameObjectByName("HitBoxAttack", gameobject);

	attackBoxTrigger = hitBoxAttackGameObject->GetComponent<ComponentBoxTrigger>();
	if (attackBoxTrigger == nullptr)
	{
		LOG("The GameObject %s has no boxTrigger component attached \n", hitBoxAttackGameObject->name);
	}

	hpHitBoxTrigger = gameobject->GetComponent<ComponentBoxTrigger>();
	if (hpHitBoxTrigger == nullptr)
	{
		LOG("The GameObject %s has no boxTrigger component attached \n", gameobject->name);
	}

	transform = gameobject->GetComponent<ComponentTransform>();
	if (transform == nullptr)
	{
		LOG("The GameObject %s has no transform component attached \n", gameobject->name);
	}
	
	GameObject* lifeUIGameObject = App->scene->FindGameObjectByName("Life");
	assert(lifeUIGameObject != nullptr);

	lifeUIComponent = lifeUIGameObject->GetComponent<ComponentImage>();
	assert(lifeUIComponent != nullptr);

	GameObject* manaUIGameObject = App->scene->FindGameObjectByName("Mana");
	assert(manaUIGameObject != nullptr);

	manaUIComponent = manaUIGameObject->GetComponent<ComponentImage>();
	assert(manaUIComponent != nullptr);

	GameObject* hubCooldownGO = nullptr;

	hubCooldownGO = App->scene->FindGameObjectByName("Q_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_Q] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_Q] != nullptr);
	}
	else
	{
		LOG("The Game Object 'Q_Cooldown' couldn't be found.");
	}


	hubCooldownGO = App->scene->FindGameObjectByName("W_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_W] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_W] != nullptr);
	}
	else
	{
		LOG("The Game Object 'W_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("E_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_E] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_E] != nullptr);
	}
	else
	{
		LOG("The Game Object 'E_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("R_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_R] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_R] != nullptr);
	}
	else
	{
		LOG("The Game Object 'R_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("One_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_1] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_1] != nullptr);
	}
	else
	{
		LOG("The Game Object '1_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Two_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_2] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_2] != nullptr);
	}
	else
	{
		LOG("The Game Object '2_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Three_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_3] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_3] != nullptr);
	}
	else
	{
		LOG("The Game Object '3_Cooldown' couldn't be found.");
	}

	hubCooldownGO = App->scene->FindGameObjectByName("Four_Cooldown");
	if (hubCooldownGO != nullptr)
	{
		hubCooldownMask[HUB_BUTTON_4] = hubCooldownGO->GetComponent<ComponentImage>();
		assert(hubCooldownMask[HUB_BUTTON_4] != nullptr);
	}
	else
	{
		LOG("The Game Object '4_Cooldown' couldn't be found.");
	}

	LOG("Started player movement script");
}

void PlayerMovement::Update()
{
	if (App->time->gameTimeScale == 0) return;

	if (health <= 0.f)
	{
		currentState = (PlayerState*)death;
		return;
	}

	PlayerState* previous = currentState;

	//Check input here and update the state!
	if (currentState != death)
	{
		for (auto it = allSkills.begin(); it != allSkills.end(); ++it) it->second->Update(App->time->gameDeltaTime);

		// Update cooldowns
		for (unsigned i = HUB_BUTTON_1; i <= HUB_BUTTON_4; ++i)
		{
			if (hubCooldownMask != nullptr && hubCooldownMask[i]->enabled)
				hubCooldownMask[i]->SetMaskAmount((int)(100.0F * hubCooldownTimer[i] / hubCooldownMax[i]));
		}
		for (unsigned i = HUB_BUTTON_Q; i <= HUB_BUTTON_R; ++i)
		{
			if (hubCooldownMask != nullptr && hubCooldownMask[i]->enabled)
				hubCooldownMask[i]->SetMaskAmount((int)(100.0F * allSkills[activeSkills[i]]->CooldownRatio()) );
		}

		currentState->UpdateTimer();

		currentState->CheckInput();

		currentState->Update();

		//if previous and current are different the functions Exit() and Enter() are called
		CheckStates(previous, currentState);	
	}

	ManaManagement();

	if (outCombatTimer > 0)
	{
		outCombatTimer -= App->time->gameDeltaTime;
	}
	else if (health < stats.health)
	{
		health += stats.hpRegen * App->time->gameDeltaTime;
		if (health > stats.health) health = stats.health;
		int healthPercentage = (health / stats.health) * 100;
		lifeUIComponent->SetMaskAmount(healthPercentage);
	}

	//Check for changes in the state to send triggers to animation SM
}

PlayerMovement_API void PlayerMovement::Damage(float amount)
{
	if (!isPlayerDead)
	{
		outCombatTimer = outCombatMaxTime;
		health -= amount;
		if (health < 0)
		{
			isPlayerDead = true;
		}

		damageController->AddDamage(gameobject->transform, amount, 5);
		damageUIFeedback->ActivateDamageUI();

		int healthPercentage = (health / stats.health) * 100;
		lifeUIComponent->SetMaskAmount(healthPercentage);
	}
}

void PlayerMovement::Equip(const PlayerStats & equipStats)
{
	this->stats += equipStats;

	int healthPercentage = (health / stats.health) * 100;
	lifeUIComponent->SetMaskAmount(healthPercentage);

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerMovement::UnEquip(const PlayerStats & equipStats)
{
	this->stats -= equipStats; 
	health = health > stats.health ? stats.health : health;
	mana = mana > stats.mana ? stats.mana : mana;

	int healthPercentage = (health / stats.health) * 100;
	lifeUIComponent->SetMaskAmount(healthPercentage);

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerMovement::Serialize(JSON_value* json) const
{
	assert(json != nullptr);
	json->AddFloat("General_Ability_Cooldown", hubGeneralAbilityCooldown);
	json->AddFloat("Q_Cooldown", hubCooldown[HUB_BUTTON_Q]);
	json->AddFloat("W_Cooldown", hubCooldown[HUB_BUTTON_W]);
	json->AddFloat("E_Cooldown", hubCooldown[HUB_BUTTON_E]);
	json->AddFloat("R_Cooldown", hubCooldown[HUB_BUTTON_R]);
	json->AddUint("Show_Ability_Cooldown", showAbilityCooldowns ? 1 : 0);
	json->AddUint("Show_Items_Cooldown", showItemCooldowns ? 1 : 0); 
	json->AddFloat("Out_of_combat_timer", outCombatMaxTime);
	
	json->AddFloat("walkingSpeed", walkingSpeed);
	json->AddFloat("MeshCorrectionXZ", OutOfMeshCorrectionXZ);
	json->AddFloat("MeshCorrectionY", OutOfMeshCorrectionY);
	json->AddFloat("MaxWalkDistance", maxWalkingDistance);

	JSON_value* keyboard_abilities = json->CreateValue();
	keyboard_abilities->AddInt("Q", (int)activeSkills[HUB_BUTTON_Q]);
	keyboard_abilities->AddInt("W", (int)activeSkills[HUB_BUTTON_W]);
	keyboard_abilities->AddInt("E", (int)activeSkills[HUB_BUTTON_E]);
	keyboard_abilities->AddInt("R", (int)activeSkills[HUB_BUTTON_R]);
	json->AddValue("keyboard_abilities", *keyboard_abilities);

	JSON_value* abilities = json->CreateValue();
	{
		JSON_value* dash_data = json->CreateValue();
		if (allSkills.find(SkillType::DASH) != allSkills.end()) allSkills.find(SkillType::DASH)->second->Serialize(dash_data);
		abilities->AddValue("dash", *dash_data);
	}
	{
		JSON_value* uppercut_data = json->CreateValue();
		if (allSkills.find(SkillType::UPPERCUT) != allSkills.end()) allSkills.find(SkillType::UPPERCUT)->second->Serialize(uppercut_data);
		abilities->AddValue("uppercut", *uppercut_data);
	}
	json->AddValue("abilities", *abilities);

	stats.Serialize(json);


}

void PlayerMovement::DeSerialize(JSON_value* json)
{
	assert(json != nullptr);
	hubGeneralAbilityCooldown = json->GetFloat("General_Ability_Cooldown", 0.5F);
	hubCooldown[HUB_BUTTON_Q] = json->GetFloat("Q_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_W] = json->GetFloat("W_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_E] = json->GetFloat("E_Cooldown", 1.0F);
	hubCooldown[HUB_BUTTON_R] = json->GetFloat("R_Cooldown", 1.0F);
	showAbilityCooldowns = json->GetUint("Show_Ability_Cooldown", 1U) == 1;
	showItemCooldowns = json->GetUint("Show_Items_Cooldown", 1U) == 1; 
	
	walkingSpeed = json->GetFloat("walkingSpeed", 300.0f);
	OutOfMeshCorrectionXZ = json->GetFloat("MeshCorrectionXZ", 500.f);
	OutOfMeshCorrectionY = json->GetFloat("MeshCorrectionY", 300.f);
	maxWalkingDistance = json->GetFloat("MaxWalkDistance", 10000.0f);

	outCombatMaxTime = json->GetFloat("Out_of_combat_timer", 3.f);

	JSON_value* keyboard_abilities = json->GetValue("keyboard_abilities");
	if (keyboard_abilities)
	{
		activeSkills[HUB_BUTTON_Q] = (SkillType)keyboard_abilities->GetInt("Q");
		activeSkills[HUB_BUTTON_W] = (SkillType)keyboard_abilities->GetInt("W");
		activeSkills[HUB_BUTTON_E] = (SkillType)keyboard_abilities->GetInt("E");
		activeSkills[HUB_BUTTON_R] = (SkillType)keyboard_abilities->GetInt("R");
	}

	JSON_value* abilities = json->GetValue("abilities");
	if (abilities)
	{
		JSON_value* dash_data = abilities->GetValue("dash");
		if (dash_data) allSkills[SkillType::DASH]->DeSerialize(dash_data, dash);

		JSON_value* uppercut_data = abilities->GetValue("uppercut");
		if (uppercut_data) allSkills[SkillType::UPPERCUT]->DeSerialize(uppercut_data, uppercut);
	}

	stats.DeSerialize(json);
}

void PlayerMovement::OnTriggerExit(GameObject* go)
{

}

bool PlayerMovement::IsAtacking() const
{
	return canInteract && App->input->GetMouseButtonDown(1) == KEY_DOWN; //Left button
}

bool PlayerMovement::IsMoving() const
{ 
	math::float3 temp;
	return (App->input->GetMouseButtonDown(3) == KEY_DOWN && canInteract|| 
			currentState->playerWalking || 
			(App->input->GetMouseButtonDown(3) == KEY_REPEAT && !App->scene->Intersects("PlayerMesh", false, temp))); //right button, the player is still walking or movement button is pressed and can get close to mouse pos
}

bool PlayerMovement::IsUsingFirstSkill() const
{
	return allSkills.find(activeSkills[HUB_BUTTON_Q])->second->IsUsable(mana) && App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondSkill() const
{
	return allSkills.find(activeSkills[HUB_BUTTON_W])->second->IsUsable(mana) &&  App->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdSkill() const
{
	return allSkills.find(activeSkills[HUB_BUTTON_E])->second->IsUsable(mana) &&  App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthSkill() const
{
	return allSkills.find(activeSkills[HUB_BUTTON_R])->second->IsUsable(mana) && App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFirstItem() const
{
	return hubCooldownTimer[HUB_BUTTON_1] <= 0 && App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN;
}

bool PlayerMovement::IsUsingSecondItem() const
{
	return hubCooldownTimer[HUB_BUTTON_2] <= 0 && App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN;
}

bool PlayerMovement::IsUsingThirdItem() const
{
	return hubCooldownTimer[HUB_BUTTON_3] <= 0 && App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN;
}

bool PlayerMovement::IsUsingFourthItem() const
{
	return hubCooldownTimer[HUB_BUTTON_4] <= 0 && App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN;
}

void PlayerMovement::UseSkill(SkillType skill)
{
	manaRegenTimer = manaRegenMaxTime;
	for (auto it = allSkills.begin(); it != allSkills.end(); ++it)
	{
		if (it->second->type == skill)
		{
			mana -= it->second->Use(hubGeneralAbilityCooldown);
		}
		else
		{
			it->second->SetCooldown(hubGeneralAbilityCooldown);
		}
	}

	for (unsigned i = 0; i < 4; ++i)
	{
		hubCooldownTimer[i] = allSkills[activeSkills[i]]->cooldown;
		hubCooldownMax[i] = allSkills[activeSkills[i]]->cooldown;
	}
}

void PlayerMovement::CheckStates(PlayerState * previous, PlayerState * current)
{
	if (previous != current)
	{
		previous->ResetTimer();

		previous->Exit();
		current->Enter();
		current->duration = anim->GetDurationFromClip();

		if (anim != nullptr)
		{
			anim->SendTriggerToStateMachine(current->trigger.c_str());
		}
	}
}

void PlayerMovement::ManaManagement()
{
	if (manaRegenTimer > 0)
	{
		manaRegenTimer -= App->time->gameDeltaTime;
	}
	else if (mana < stats.mana && outCombatTimer <= 0)
	{
		mana += stats.manaRegen * App->time->gameDeltaTime;
		if (mana > stats.mana) mana = stats.mana;
	}

	int manaPercentage = (mana / stats.mana) * 100;
	manaUIComponent->SetMaskAmount(manaPercentage);
}

void PlayerStats::Serialize(JSON_value * json) const
{
	JSON_value* statsValue = json->CreateValue();

	statsValue->AddFloat("health", health);
	statsValue->AddFloat("mana", mana);
	statsValue->AddInt("strength", strength);
	statsValue->AddInt("dexterity", dexterity);
	statsValue->AddFloat("hp_regen", hpRegen);
	statsValue->AddFloat("mana_regen", manaRegen);

	json->AddValue("stats", *statsValue);
}

void PlayerStats::DeSerialize(JSON_value * json)
{

	JSON_value* statsValue = json->GetValue("stats");
	if (!statsValue) return;

	health = statsValue->GetFloat("health", 100.0F);
	mana = statsValue->GetFloat("mana", 100.0F);
	strength = statsValue->GetInt("strength", 10);
	dexterity = statsValue->GetInt("dexterity", 10);
	hpRegen = statsValue->GetFloat("hp_regen", 5.0F);
	manaRegen = statsValue->GetFloat("mana_regen", 5.0F);
}

void PlayerStats::Expose(const char* sectionTitle)
{
	ImGui::Text(sectionTitle);
	ImGui::InputFloat("Health", &health);
	ImGui::InputFloat("Mana", &mana);
	
	int uiStrength = (int)strength;
	if (ImGui::InputInt("Strength", &uiStrength)) strength = uiStrength < 0 ? 0 : uiStrength;
	
	int uiDexterity = (int)dexterity;
	if (ImGui::InputInt("Strength", &uiDexterity)) dexterity = uiDexterity < 0 ? 0 : uiDexterity;
	
	ImGui::DragFloat("HP regen", &hpRegen, 1.0F, 0.0F, 10.0F);
	ImGui::DragFloat("Mana regen", &manaRegen, 1.0F, 0.0F, 10.0F);
}

void PlayerMovement::ActivateHudCooldownMask(bool activate, unsigned first, unsigned last)
{
	for (unsigned i = first; i <= last; ++i) hubCooldownMask[i]->gameobject->SetActive(activate);
}

void PlayerSkill::Expose(const char * title)
{
	bool open = true;
	{
		ImGui::PushID(title);
		ImGui::Bullet(); ImGui::SameLine(); ImGui::Text(title);
		ImGui::DragFloat("Mana Cost", &manaCost);
		ImGui::DragFloat("Cooldown", &this->cooldown);
		ImGui::Text("Timer: %f (%f)", cooldownTimer, CooldownRatio());
		ImGui::PopID();
	}
}

void PlayerSkill::Serialize(JSON_value * json) const
{
	json->AddInt("type", (int)type); 
	json->AddFloat("mana_cost", manaCost);
	json->AddFloat("cooldown", cooldown);
}

void PlayerSkill::DeSerialize(JSON_value * json, PlayerState * playerState)
{
	//type = (SkillType)json->GetInt("type"); 
	manaCost = json->GetFloat("mana_cost"); 
	cooldown = json->GetFloat("cooldown"); 
	state = playerState;
}
