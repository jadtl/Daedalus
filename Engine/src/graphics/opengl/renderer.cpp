#include "graphics/opengl/renderer.h"

#include "core/log.h"
#include "core/resources.h"

#include <glad/glad.h>
#include <stb_image.h>
extern "C" {
#include "lua.h"
}
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ddls::gl {

static void resizeCallback(GLFWwindow *window, i32 width, i32 height);
static f32 Vertices[] = {
	// positions          // texture coords
	 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
	 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
	-0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
};
static u32 Indices[] = {
    0, 1, 3,
	1, 2, 3
};  

Renderer::Renderer(const RendererConfig& config): ddls::Renderer(config)
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DDLS_PLATFORM_MACOS
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GL_TRUE);
#endif

	_window = glfwCreateWindow(
		_config.width, 
		_config.height, 
		_config.applicationName.c_str(), 
		nullptr, 
		nullptr
	);
	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	i32 width, height;
	glfwGetFramebufferSize(_window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(_window, resizeCallback);
	glfwSetWindowUserPointer(_window, this);

	_pipeline = new Pipeline("shader.vert.glsl", "shader.frag.glsl");

	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	glGenBuffers(1, &_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(3 * sizeof(f32)));
    glEnableVertexAttribArray(1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	camera = Camera();
	projectionUpdate();

	// Text render state initialization
	glGenVertexArrays(1, &_VAOText);
	glGenBuffers(1, &_VBOText);
	glBindVertexArray(_VAOText);
    glBindBuffer(GL_ARRAY_BUFFER, _VBOText);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	_pipelineText = new Pipeline("shader.text.vert.glsl", "shader.text.frag.glsl");
	_projectionText = ortho(0.0f, (f32)_config.width, 0.0f, (f32)_config.height);
	_pipelineText->bind();
	_pipelineText->setMat4("projection", _projectionText);
	_pipelineText->setInt("text", 0);
}

Renderer::~Renderer()
{
	delete _pipeline;
	glDeleteVertexArrays(1, &_VAO);
	glDeleteBuffers(1, &_VBO);
	glDeleteBuffers(1, &_EBO);
	glDeleteVertexArrays(1, &_VAOText);
	glDeleteBuffers(1, &_VBOText);
}

void Renderer::newFrame()
{
	glfwSwapBuffers(_window);
}

void Renderer::clear(vec3 color)
{
	glClearColor(color.x, color.y, color.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::loadTexture(const char* texture)
{
	_textures[texture].load(Resources::Manager().getTexture(texture));
}

void Renderer::drawTexture(const char* texture, mat4 model)
{
	if (!_textures.count(texture)) loadTexture(texture);

	glActiveTexture(GL_TEXTURE0);
	_pipeline->setInt("tex", 0);
	_textures[texture].bind();

	_pipeline->bind();
	_pipeline->setMat4("model", model);
	_pipeline->setMat4("view", camera.view());
	_pipeline->setMat4("projection", _projection);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::loadFont(const char* fontName)
{
	FT_Library ft;
	FT_Init_FreeType(&ft);

	FT_Face face;
	std::filesystem::path path = Resources::Manager().getPath(fontName);
	if(FT_New_Face(ft, path.c_str(), 0, &face))
	{
		Log::Error("Failed to load font at ", path);
	}

	// set size to load glyphs as
	FT_Set_Pixel_Sizes(face, 0, 96);

	// disable byte-alignment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// load first 128 characters of ASCII set
	for (unsigned char c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			Log::Error("Failed to load Glyph");
			continue;
		}
		// generate texture
		u32 texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			(GLsizei)face->glyph->bitmap.width,
			(GLsizei)face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// now store character for later use
		Character character = {
			texture,
			ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<u32>(face->glyph->advance.x)
		};
		_characters.insert(std::pair<char, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void Renderer::drawText(std::string text, vec2 position, float scale, vec3 color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	_pipelineText->bind();
	_pipelineText->setVec3("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(_VAOText);

	// iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = _characters[*c];

        float xpos = position.x + ch.bearing.x * scale;
        float ypos = position.y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texId);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, _VBOText);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        position.x += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
}

void Renderer::viewportUpdate(u16 width, u16 height)
{
	_config.width = width;
	_config.height = height;
	projectionUpdate();
	glViewport(0, 0, width, height);
	_projectionText = ortho(0.0f, (f32)width, 0.0f, (f32)height);
	_pipelineText->bind();
	_pipelineText->setMat4("projection", _projectionText);
}

static void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
	ddls::ignore(window);
	ddls::Renderer *renderer = (ddls::Renderer *)glfwGetWindowUserPointer(window);
	renderer->viewportUpdate((u16)width, (u16)height);
}

} // namespace ddls::gl
