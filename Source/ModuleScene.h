#ifndef __ModuleScene_h__
#define __ModuleScene_h__

#include "Module.h"
#include "Geometry/Frustum.h"
#include "pcg_random.hpp"
#include "Math/Quat.h"
#include "Math/float4.h"

class GameObject;
class ComponentCamera;
class ComponentLight;
struct Texture;
struct par_shapes_mesh_s;

class ModuleScene :
	public Module
{
public:
	ModuleScene();
	~ModuleScene();

	bool Init() override;
	bool Start() override;

	update_status Update() override;
	void Draw(const math::Frustum &frustum);
	void DrawHierarchy();

	GameObject * CreateGameObject(const float4x4& transform, const char* filepath, const char* name, GameObject* parent);
	GameObject * CreateGameObject(const char * name, GameObject* parent);

	void CreateCube(const char * name, const float3 & pos = float3::zero, const Quat & rot = Quat::identity, float size = 1.f, const float4 & color = float4::one);
	void CreateSphere(const char * name, const float3 & pos = float3::zero, const Quat & rot = Quat::identity, float size = 1.f, unsigned slices = 20u, unsigned stacks = 20u, const float4 & color = float4::one);
	void CreatePrimitive(par_shapes_mesh_s *mesh, const char * name, const float3 & pos, const Quat & rot, float size, const float4 & color);
	unsigned SaveParShapesMesh(const par_shapes_mesh_s & mesh, char** data) const;
	void SaveScene(const GameObject &rootGO, const char* name) const;
	void LoadScene(const char * scene);
	unsigned GetNewUID();

public:
	GameObject* root = nullptr;
	GameObject* selected = nullptr;
	ComponentCamera* maincamera = nullptr; //Released by GameObject holding it
	Texture* camera_notfound_texture = nullptr; //Released in resource manager

	ComponentLight* light = nullptr;//Released by GameObject holding it

	pcg32 uuid_rng;
	std::string name;
};

#endif __ModuleScene_h__