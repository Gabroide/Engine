#include "EnemyLifeBarController.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentImage.h"

EnemyLifeBarController_API Script* CreateScript()
{
	EnemyLifeBarController* instance = new EnemyLifeBarController;
	return instance;
}

void EnemyLifeBarController::Start()
{
	GameObject* enemyLife = App->scene->FindGameObjectByName("EnemyLife");
	lifeBackground = App->scene->FindGameObjectByName(enemyLife, "LifeBackground");
	hPbar = App->scene->FindGameObjectByName(enemyLife, "HPbar");
	enemyTypeName = App->scene->FindGameObjectByName(enemyLife, "EnemyTypeName");
	boneRight = App->scene->FindGameObjectByName(enemyLife, "BoneRight");
	boneLeft = App->scene->FindGameObjectByName(enemyLife, "BoneLeft");
	skull = App->scene->FindGameObjectByName(enemyLife, "Skull");

	lifeImage = hPbar->GetComponent<ComponentImage>();
}

void EnemyLifeBarController::Update()
{
	lifeBackground->SetActive(false);
	hPbar->SetActive(false);
	enemyTypeName->SetActive(false);
	skull->SetActive(false);
	boneRight->SetActive(false);
	boneLeft->SetActive(false);
}

void EnemyLifeBarController::SetLifeBar(int maxHP, int actualHP, EnemyLifeBarType type, std::string name)
{
	lifeBackground->SetActive(true);
	hPbar->SetActive(true);
	enemyTypeName->SetActive(true);
	switch (type)
	{
	case EnemyLifeBarType::NORMAL:
		skull->SetActive(false);
		boneRight->SetActive(false);
		boneLeft->SetActive(false);
		break;
	case EnemyLifeBarType::HARD:
		skull->SetActive(true);
		boneRight->SetActive(false);
		boneLeft->SetActive(false);
		break;
	case EnemyLifeBarType::BOSS:
		skull->SetActive(true);
		boneRight->SetActive(true);
		boneLeft->SetActive(true);
		break;
	default:
		break;
	}

	lifeImage->SetMaskAmount((actualHP * 100) / maxHP);
}
