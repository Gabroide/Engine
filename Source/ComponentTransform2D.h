#ifndef __ComponentTransform2D_h__
#define __ComponentTransform2D_h__

#include "Component.h"
#include "Math/float2.h"
#include <vector>

class ComponentTransform2D : public Component
{
public:
	ComponentTransform2D(GameObject* gameobject);
	ComponentTransform2D(const ComponentTransform2D& component);
	~ComponentTransform2D();

	Component* Clone() const override;
	void DrawProperties() override;
	bool CleanUp() override;

	void Save(JSON_value* value) const override;
	void Load(JSON_value* value) override;
	ENGINE_API math::float2 getPosition() const;
	ENGINE_API void setPosition(const math::float2& position);

	math::float2 getSize() const;
private:
	enum aligns {
		TOPLEFT = 0,
		TOPCENTER = 1,
		TOPRIGHT,
		MIDDLELEFT,
		MIDDLECENTER,
		MIDDLERIGHT,
		BOTTOMLEFT,
		BOTTOMCENTER,
		BOTTOMRIGHT
	};

	//variables
	int currentAnchor = MIDDLECENTER;
	std::vector<float2> alignments = std::vector<float2>(9);

	math::float2 position = math::float2::zero;
	math::float2 size = math::float2(100.0f, 100.0f);
};

#endif

