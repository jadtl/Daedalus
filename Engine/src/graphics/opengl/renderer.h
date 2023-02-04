#pragma once

#include "graphics/renderer.h"

namespace ddls {
namespace gl {
class DDLS_API Renderer : public ddls::Renderer
{
public:
	Renderer(RendererConfig config);
	~Renderer();
	void newFrame();
	void draw();
	void resizeCallback(u16 width, u16 height);
	void setViewport(vec4 rect);
	void resetViewport();
};
}
}
