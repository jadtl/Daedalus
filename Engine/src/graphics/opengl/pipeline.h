#pragma once

#include "core/defines.h"
#include "core/types.h"

#include <string>

namespace ddls::gl {

class DDLS_API Pipeline
{
public:
	Pipeline(const char* vertexShaderPath, const char* fragmentShaderPath);
	~Pipeline();

	/**
	 * @brief The handle to the API-specific object that uses the shaders
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
	void setBoolean(const std::string &uniform, b8  value) const;
	void setInteger(const std::string &uniform, i32 value) const;
	void setFloat  (const std::string &uniform, f32 value) const;

private:
	u32 _handle;
};

} // namespace ddls::gl
