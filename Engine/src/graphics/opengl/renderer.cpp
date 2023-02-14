#include "graphics/opengl/renderer.h"

#include <core/log.h>

#include <glad/glad.h>
#include <stb_image.h>
#include <lua.hpp>

namespace ddls::gl {

static void resizeCallback(GLFWwindow *window, i32 width, i32 height);
static f32 Vertices[] = {
    // positions         // colors
     0.5f, -0.5f, 0.0f,  0.5f, 0.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
     0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
};   
static u32 Indices[] = {
    0, 1, 2,
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

	_pipeline = new Pipeline(config.vertexShader, config.fragmentShader);

	glGenVertexArrays(1, &_VAO);
	glBindVertexArray(_VAO);

	glGenBuffers(1, &_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

Renderer::~Renderer()
{
	delete _pipeline;
	glDeleteVertexArrays(1, &_VAO);
	glDeleteBuffers(1, &_VBO);
}

static f32 red;

void Renderer::newFrame()
{
	glfwSwapBuffers(_window);
	f32 time = (f32)glfwGetTime();
	red = (sin(time) / 2.0f) + 0.5f;
}

void Renderer::draw()
{
	glClearColor(0.5f, 0.0f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	_pipeline->bind();

	i32 colorLocation = glGetUniformLocation(_pipeline->handle(), "offset");
	glUniform4f(colorLocation, red, 0.0f, 0.0f, 1.0f);
	Log::Debug(red);

	glBindVertexArray(_VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
	ddls::ignore(window);
	glViewport(0, 0, width, height);
}

} // namespace ddls::gl
