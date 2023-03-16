#pragma once

#include "core/defines.h"
#include "core/types.h"

#include <glm/glm.hpp>

#include <string>

using namespace glm;
namespace ddls::gl {

class DDLS_API Pipeline
{
public:
	Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath);
	~Pipeline();

	/**
	 * @brief The handle to the shader program
	 *
	 */
	u32 handle() const { return _handle; };

	/**
	 * @brief Binds the pipeline to the running program
	 *
	 */
	void bind() const;

	/**
	 * @brief Uniform utility functions
	 *
	 */
	void setBool (const std::string &uniform, b8  value) const;
	void setInt  (const std::string &uniform, i32 value) const;
	void setFloat(const std::string &uniform, f32 value) const;
    void setVec2 (const std::string &uniform, const vec2 &value) const;
    void setVec2 (const std::string &uniform, float x, float y) const;
    void setVec3 (const std::string &uniform, const vec3 &value) const;
    void setVec3 (const std::string &uniform, float x, float y, float z) const;
    void setVec4 (const std::string &uniform, const vec4 &value) const;
    void setVec4 (const std::string &uniform, float x, float y, float z, float w) const;
    void setMat2 (const std::string &uniform, const mat2 &mat) const;
    void setMat3 (const std::string &uniform, const mat3 &mat) const;
    void setMat4 (const std::string &uniform, const mat4 &mat) const;

private:
	u32 _handle;
};

} // namespace ddls::gl
