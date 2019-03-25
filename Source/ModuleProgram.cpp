#include "Application.h"

#include "ModuleProgram.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"

#include "GL/glew.h"
#include <assert.h>

Shader::~Shader()
{
	if (id[0] != 0)
	{
		glDeleteProgram(id[0]);
	}
}

ModuleProgram::ModuleProgram()
{
}


ModuleProgram::~ModuleProgram()
{
}

bool ModuleProgram::Init(JSON * config)
{
	defaultShader = CreateProgram(DEFAULTPROGRAM);
	return true;
}



Shader* ModuleProgram::CreateProgram(const char* name)
{
	assert(name != nullptr);
	if (name == nullptr) return nullptr;

	unsigned int vertexShader = CreateVertexShader(name);
	unsigned int fragmentShader = CreateFragmentShader(name);

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int  success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		LOG("ERROR::PROGRAM::CREATION_FAILED\n");
		LOG("ERROR: %s\n", infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	Shader* shader = new Shader(program, name);
	App->resManager->AddProgram(shader);
	return shader;
	
}

Shader* ModuleProgram::GetProgram(const char* name)
{
	std::string vName(name);
	
	Shader* shader = App->resManager->GetProgram(vName);
	if (shader != nullptr)
	{
		App->resManager->AddProgram(shader); //Add to number of usages
		return shader;
	}
	
	if (std::string(name) == "PBR")
	{		
		return CreatePBRVariations(name);
	}
	else
	{
		return CreateProgram(name);
	}
}

unsigned ModuleProgram::CreateVertexShader(const char *name)
{
	assert(name != NULL);
	std::string file(name);
	file += ".vs";
	char * vertexShaderSource = nullptr;
	App->fsystem->Load((VERTEXSHADERS + file).c_str(), &vertexShaderSource);

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	RELEASE_ARRAY(vertexShaderSource);

	ShaderLog(vertexShader, "VERTEX");
	return vertexShader;
}

unsigned ModuleProgram::CreateFragmentShader(const char *name)
{
	assert(name != NULL);
	std::string file(name);
	file += ".fs";
	char * fragmentShaderSource = nullptr;
	App->fsystem->Load((FRAGSHADERS+file).c_str(), &fragmentShaderSource);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	RELEASE_ARRAY(fragmentShaderSource);

	ShaderLog(fragmentShader, "FRAGMENT");
	return fragmentShader;
}

Shader* ModuleProgram::CreatePBRVariations(const char* name)
{
	assert(name != NULL);
	Shader* shader = new Shader(0, name);
	unsigned variationAmount = pow(2, PBR_VARIATIONS - 1);
	for (unsigned variation = 0u; variation < variationAmount; ++variation)
	{
		std::string file(name);
		file += ".vs";
		char* vertexShaderSource = nullptr;
		App->fsystem->Load((VERTEXSHADERS + file).c_str(), &vertexShaderSource);

		file = std::string(name);
		file += ".fs";
		char* fragmentShaderSource = nullptr;
		App->fsystem->Load((FRAGSHADERS + file).c_str(), &fragmentShaderSource);

		char** vs = new char*[PBR_VARIATIONS + 1];
		vs[0] = "#version 330\n";
		char** fs = new char*[PBR_VARIATIONS + 1];
		fs[0] = "#version 330\n";
		unsigned index = 1u;
		for (unsigned i = 0; i < PBR_VARIATIONS; ++i)
		{
			if ((variation & (1 << i)) != 0)
			{
				vs[index] = defines[i];
				fs[index] = defines[i];
				++index;
			}
		}
		vs[index] = vertexShaderSource;
		fs[index] = fragmentShaderSource;
		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, index + 1, vs, nullptr);
		glCompileShader(vertexShader);
		RELEASE_ARRAY(vs);
		RELEASE_ARRAY(vertexShaderSource);

		ShaderLog(vertexShader, "VERTEX");

		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, index + 1, fs, nullptr);
		glCompileShader(fragmentShader);
		RELEASE_ARRAY(fs);
		RELEASE_ARRAY(fragmentShaderSource);

		ShaderLog(fragmentShader, "FRAGMENT");

		unsigned int program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);

		int  success;
		char infoLog[512];
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			LOG("ERROR::PROGRAM::CREATION_FAILED\n");
			LOG("ERROR: %s\n", infoLog);
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		shader->id[variation] = program;
	}
	App->resManager->AddProgram(shader);
	return shader;
}

void ModuleProgram::ShaderLog(unsigned int shader, char * type) const
{
	assert(type != NULL);
	int  success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		LOG("ERROR::SHADER::%s::COMPILATION_FAILED\n", type);
		LOG("ERROR: %s\n", infoLog);
	}
}

bool ModuleProgram::CleanUp()
{
	if (defaultShader != nullptr)
	{
		App->resManager->DeleteProgram(defaultShader->file);
		defaultShader == nullptr;
	}
	return true;
}
