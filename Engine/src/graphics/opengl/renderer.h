#pragma once

#include "graphics/renderer.h"

#include "graphics/opengl/pipeline.h"
#include "graphics/opengl/texture.h"

namespace ddls::gl {

struct Character
{
	u32 texId;
	ivec2 size;
	ivec2 bearing;
	u32 advance;
};

class DDLS_API Renderer : public ddls::Renderer
{
public:
	explicit Renderer(const RendererConfig& config);
	~Renderer() override;
	void newFrame() override;
	void clear(vec3 color) override;
	void loadTexture(const char* texture) override;
	void drawTexture(const char* texture, mat4 model) override;
	void loadFont(const char* fontName) override;
	void drawText(std::string text, vec2 position, float scale, vec3 color) override;
	void viewportUpdate(u16 width, u16 height) override;

private:
	// Textures
	Pipeline *_pipeline;
	u32 _VAO{};
	u32 _VBO{};
	u32 _EBO{};
	std::map<const char*, Texture> _textures;

	// Text
	Pipeline *_pipelineText;
	u32 _VAOText{};
	u32 _VBOText{};
	std::map<char, Character> _characters;
	mat4 _projectionText;
};

} // namespace ddls::gl
