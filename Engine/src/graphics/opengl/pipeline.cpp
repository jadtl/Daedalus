#include "graphics/opengl/pipeline.h"

#include "core/resources.h"
#include "core/log.h"

#include <glad/glad.h>

namespace ddls {
namespace gl {

static u32 createShader(const char* filePath, GLenum type);
static char* readFile(const char *fileName);

Pipeline::Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath)
{
	u32 vertexShader = createShader(vertexShaderPath, GL_VERTEX_SHADER);
	u32 fragmentShader = createShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

	_handle = glCreateProgram();
	glAttachShader(_handle, vertexShader);
	glAttachShader(_handle, fragmentShader);
	glLinkProgram(_handle);

	i32 success;
	char infoLog[512];
	glGetProgramiv(_handle, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(_handle, 512, NULL, infoLog);
		Log::Error("Vertex shader compilation failed: ", infoLog);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Pipeline::~Pipeline()
{
	glDeleteProgram(_handle);
}

void Pipeline::bind()
{
	glUseProgram(_handle);
}

void Pipeline::setBoolean(const std::string &uniform, b8  value) const
{
	glUniform1i(glGetUniformLocation(_handle, uniform.c_str()), (i32)value);
}

void Pipeline::setInteger(const std::string &uniform, i32 value) const
{
	glUniform1i(glGetUniformLocation(_handle, uniform.c_str()), value);
}

void Pipeline::setFloat  (const std::string &uniform, f32 value) const
{
	glUniform1f(glGetUniformLocation(_handle, uniform.c_str()), value);
}

static u32 createShader(const char* filePath, GLenum type)
{
	const char* shaderSrc = Resources::Manager().get(filePath);

	u32 shader = glCreateShader(type);

	glShaderSource(shader, 1, &shaderSrc, nullptr);
	glCompileShader(shader);

	Resources::Manager().free(filePath);

	i32 success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		Log::Error("Vertex shader compilation failed: ", infoLog);
	}

	return shader;
}

} // namespace gl
} // namespace ddls
