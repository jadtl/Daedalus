#pragma once

#include "core/defines.h"
#include "core/types.h"
#include "core/resources.h"
#include "graphics/camera.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <utility>

using namespace glm;

namespace ddls {

enum ProjectionType
{
	Orthographic,
	Perspective
};

struct DDLS_API RendererConfig
{
	std::string applicationName;
	
	u16 width;
	u16 height;

	ProjectionType projectionType;
	f32 planeNear;
	f32 planeFar;
};

class DDLS_API Renderer
{
public:
	explicit Renderer(RendererConfig config) : _config(std::move(config)) {};
	virtual ~Renderer() = default;
	virtual void newFrame() = 0;
	virtual void clear(vec3 color) = 0;
	virtual void loadTexture(const char* texture) = 0;
	virtual void drawTexture(const char* texture, mat4 model) = 0;
	virtual void loadFont(const char* fontName) = 0;
	virtual void drawText(std::string text, vec2 position, float scale, vec3 color) = 0;
	virtual void viewportUpdate(u16 width, u16 height) = 0;

	GLFWwindow *window() { return _window; }

	void projectionUpdate();

	Camera camera;
protected:
	RendererConfig _config;
	GLFWwindow *_window;
	mat4 _projection;
};

}