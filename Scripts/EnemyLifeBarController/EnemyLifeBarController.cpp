#include "EnemyLifeBarController.h"

#include "Application.h"
#include "ModuleScene.h"

#include "GameObject.h"
#include "ComponentTransform2D.h"

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

	life = hPbar->GetComponent<Transform2D>();
	lifeBarMaxWidht = life->getSize();
	lifeBarPositionX = life->getPosition();
}

void EnemyLifeBarController::Update()
{
	lifeBackground->SetActive(false);
	hPbar->SetActive(false);
	enemyTypeName->SetActive(false);
	skull->SetActive(false);
	boneRight->SetActive(false);
	boneLeft->SetActive(false);
	life->SetSize(lifeBarMaxWidht);
	life->SetPositionUsingAligment(lifeBarPositionX);
}

void EnemyLifeBarController::SetLifeBar(int maxHP, int actualHP, EnemyLifeBarType type, std::string name)
{
	life->SetSize(lifeBarMaxWidht);
	life->SetPositionUsingAligment(lifeBarPositionX);
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

	float actualWidth = (actualHP * life->getSize().x) / maxHP;

	float difference = life->getSize().x - actualWidth;

	life->SetSize(math::float2(life->getSize().x - difference, life->getSize().y));
	math::float2 newPosition = math::float2(life->getPosition().x - (difference / 2), life->getPosition().y);
	life->SetPositionUsingAligment(newPosition);
}
