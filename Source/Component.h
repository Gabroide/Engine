#ifndef __Component_h__
#define __Component_h__

#include "Globals.h"
class GameObject;
class JSON_value;

enum class ComponentType 
{
	Transform = 0,
	Renderer,
	Camera,
	Light,
	Animation,
	Transform2D,
	Text,
	Image,
	Button,
	Script
};

class Component
{
public:
	Component(GameObject* gameobject, ComponentType type);
	Component(const Component& component);

	virtual ~Component();
	virtual Component* Clone() const  = 0;
	
	bool DrawComponentState();

	virtual void DrawProperties() = 0;

	ENGINE_API virtual void Enable(bool enable)
	{
		enabled = enable;
	}

	virtual void Update() {}
	
	virtual bool CleanUp() 
	{
		return true; 
	}

	virtual void Options();
	void Remove();

	virtual void Copy();
	virtual void Paste() {}
	virtual void Reset() {}

	virtual void Save(JSON_value* value) const;
	virtual void Load(JSON_value* value);

public:
	GameObject* gameobject = nullptr;
	ComponentType type;
	bool enabled = true;
};

#endif __Component_h__