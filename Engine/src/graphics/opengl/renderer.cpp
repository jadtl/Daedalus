#include "graphics/opengl/renderer.h"

#include <core/log.h>

#include <glad/glad.h>


namespace ddls {
static void resizeCallback(GLFWwindow *window, i32 width, i32 height);
float vertices[] =
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};
namespace gl {

Renderer::Renderer(RendererConfig config): ddls::Renderer(config)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DDLS_PLATFORM_MACOS
	glfwWindowHint(GLFW_OPENGL_COMPAT_PROFILE, GL_TRUE);
#endif
	glfwMakeContextCurrent(_config.window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, _config.width, _config.height);
	glfwSetFramebufferSizeCallback(_config.window, ddls::resizeCallback);

	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

Renderer::~Renderer()
{

}

void Renderer::newFrame()
{
	glfwSwapBuffers(_config.window);
}

void Renderer::draw()
{
	glClearColor(0.5f, 0.0f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::resizeCallback(u16 width, u16 height)
{

}

void Renderer::setViewport(vec4 rect)
{

}

void Renderer::resetViewport()
{

}

} // namespace gl

static void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
	ddls::ignore(window);
	glViewport(0, 0, width, height);
}

} // namespace ddls

