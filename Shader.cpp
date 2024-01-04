#include "Shader.h"

#include <iostream>
#include <glad/glad.h>

void Shader::CheckCompileErrors(unsigned shaderStencilTesting, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") 
	{
		glGetShaderiv(shaderStencilTesting, GL_COMPILE_STATUS, &success);
		if (!success) 
		{
			glGetShaderInfoLog(shaderStencilTesting, 1024, NULL, infoLog);
			std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else {
		glGetProgramiv(shaderStencilTesting, GL_LINK_STATUS, &success);
		if (!success) 
		{
			glGetProgramInfoLog(shaderStencilTesting, 1024, NULL, infoLog);
			std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
