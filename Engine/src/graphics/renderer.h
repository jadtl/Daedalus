#pragma once

#include "core/defines.h"
#include "core/types.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

using namespace glm;

namespace ddls {
struct DDLS_API RendererConfig
{
	std::string applicationName;
	
	u16 width;
	u16 height;

	GLFWwindow *window;
};

class DDLS_API Renderer
{
public:
	Renderer(RendererConfig config) : _config(config) {};
	virtual void newFrame() = 0;
	virtual void draw() = 0;
	virtual void resizeCallback(u16 width, u16 height) = 0;
	virtual void setViewport(vec4 rect) = 0;
	virtual void resetViewport() = 0;
protected:
	RendererConfig _config;
};
}
