#ifndef  __PlayerMovement_h__
#define  __PlayerMovement_h__

class GameObject;
class JSON_value;
struct ImGuiContext;
#include "BaseScript.h"
#include "Math/float3.h"
#include <vector>

#ifdef PlayerMovement_EXPORTS
#define PlayerMovement_API __declspec(dllexport)

#else
#define PlayerMovement_API __declspec(dllimport)
#endif

class ComponentAnimation;
class JSON_value;
struct ImGuiContext;

enum class PlayerState
{
	IDLE,
	WALK,
	ATTACK
};

class PlayerMovement_API PlayerMovement : public Script
{
public:
	void Expose(ImGuiContext* context) override;

	void Start() override;
	void Update() override;
	void Serialize(JSON_value* json) const override;
	void DeSerialize(JSON_value* json) override;

	void CheckState(PlayerState previous, PlayerState current);
	void Damage(float amount);
public:
	bool isPlayerDead = false;
	float3 currentPosition = float3(0, 0, 0); //TODO ZERO
	PlayerState playerState = PlayerState::IDLE;
private:
	int pathIndex = 0;
	std::vector<float3>path;
	
	float speed = 2.0f;
	float health = 100.0f;
	float attackDuration = 1.0f;
	float attackTimer = 0.0f;
	ComponentAnimation* anim = nullptr;
};
#endif __PlayerMovement_h__
