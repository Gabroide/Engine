#ifndef __ModuleResourceManager_h__
#define __ModuleResourceManager_h__

#include "Module.h"
#include <map>
#include <list>

struct Texture;
struct Shader;

class ModuleResourceManager :
	public Module
{
public:
	ModuleResourceManager();
	~ModuleResourceManager();

	Texture * GetTexture(std::string filename) const;

	void AddTexture(std::string filename, Texture * texture);

	void DeleteTexture(std::string filename);

	Shader* GetProgram(std::string filename) const;
	std::list<Shader*>GetAllPrograms() const;

	void AddProgram(Shader* shader);

	void DeleteProgram(std::string filename);

private:
	std::map<std::string, std::pair<unsigned, Texture*>> textureResources; //filename , times used, texture pointer
	std::map<std::string, std::pair<unsigned, Shader*>> shaderResources; //filename , times used, shader
};

#endif __ModuleResourceManager_h__