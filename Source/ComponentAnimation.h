#ifndef __COMPONENTANIMATION_H_
#define __COMPONENTANIMATION_H_

#include "Component.h"
#include "Animation.h"
#include "AnimationController.h"

#include "Math/float4x4.h"
#include "Math/float3.h"
#include "Math/Quat.h"

class ComponentAnimation : public Component
{
public:

	void DrawProperties();

	Component* Clone() const;
	void UpdateGO(GameObject* gameobject);

	ComponentAnimation();
	ComponentAnimation(GameObject* gameobject);
	ComponentAnimation(const ComponentAnimation &copy);
	~ComponentAnimation();

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;

	void Update() override;
	bool CleanUp();
public:

	AnimationController* controller = nullptr;
	Animation* anim = nullptr;
};

#endif //  __COMPONENTANIMATION_H_
