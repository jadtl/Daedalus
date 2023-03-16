#include "graphics/opengl/pipeline.h"

#include "core/resources.h"
#include "core/log.h"

#include <glad/glad.h>

namespace ddls::gl {

static u32 createShader(const char *filePath, GLenum type);

Pipeline::Pipeline(const char *vertexShaderPath, const char *fragmentShaderPath)
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

    if (!success) {
        glGetProgramInfoLog(_handle, 512, NULL, infoLog);
        Log::Error("Program linking error: ", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Pipeline::~Pipeline()
{
    glDeleteProgram(_handle);
}

void Pipeline::bind() const
{
    glUseProgram(_handle);
}

void Pipeline::setBool(const std::string &uniform, b8 value) const
{
    glUniform1i(glGetUniformLocation(_handle, uniform.c_str()), (i32) value);
}
void Pipeline::setInt(const std::string &uniform, i32 value) const
{
    glUniform1i(glGetUniformLocation(_handle, uniform.c_str()), value);
}
void Pipeline::setFloat(const std::string &uniform, f32 value) const
{
    glUniform1f(glGetUniformLocation(_handle, uniform.c_str()), value);
}
void Pipeline::setVec2(const std::string &name, const vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]); 
}
void Pipeline::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(_handle, name.c_str()), x, y); 
}
void Pipeline::setVec3(const std::string &name, const vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]); 
}
void Pipeline::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(_handle, name.c_str()), x, y, z); 
}
void Pipeline::setVec4(const std::string &name, const vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(_handle, name.c_str()), 1, &value[0]); 
}
void Pipeline::setVec4(const std::string &name, float x, float y, float z, float w) const
{ 
    glUniform4f(glGetUniformLocation(_handle, name.c_str()), x, y, z, w); 
}
void Pipeline::setMat2(const std::string &name, const mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Pipeline::setMat3(const std::string &name, const mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Pipeline::setMat4(const std::string &name, const mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(_handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

static u32 createShader(const char *filePath, GLenum type)
{
    const char *shaderSrc = Resources::Manager().getFile(filePath);

    u32 shader = glCreateShader(type);

    glShaderSource(shader, 1, &shaderSrc, nullptr);
    glCompileShader(shader);

    Resources::Manager().free(filePath);

    i32 success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        Log::Error("Vertex shader compilation failed: ", infoLog);
    }

    return shader;
}

} // namespace ddls::gl
