#pragma once

#include "graphics/renderer.h"

#include "graphics/opengl/pipeline.h"

namespace ddls::gl {
class DDLS_API Renderer : public ddls::Renderer
{
public:
	explicit Renderer(const RendererConfig& config);
	~Renderer() override;
	void newFrame() override;
	void draw() override;
private:
	//u32 _shaderProgram;
	Pipeline *_pipeline;
	u32 _VAO{};
	u32 _VBO{};
	u32 _EBO{};
};
} // namespace ddls::gl
