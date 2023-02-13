#pragma once

#include "graphics/renderer.h"

#include "graphics/opengl/pipeline.h"

namespace ddls {
namespace gl {
class DDLS_API Renderer : public ddls::Renderer
{
public:
	Renderer(RendererConfig config);
	~Renderer();
	void newFrame();
	void draw();
private:
	//u32 _shaderProgram;
	Pipeline *_pipeline;
	u32 _VAO;
	u32 _VBO;
	u32 _EBO;
};
}
}
