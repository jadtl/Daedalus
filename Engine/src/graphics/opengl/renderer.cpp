#include "graphics/opengl/renderer.h"

#include <core/log.h>

#include <glad/glad.h>


namespace ddls {
static u32 createShader(const char* fileName, u32 type);
static char* readFile(const char *fileName);
static void resizeCallback(GLFWwindow *window, i32 width, i32 height);
static f32 vertices[] =
{
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};
namespace gl {

Renderer::Renderer(RendererConfig config): ddls::Renderer(config)
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
	glfwSetFramebufferSizeCallback(_window, ddls::resizeCallback);

	glGenVertexArrays(1, &_VAO);
	glGenBuffers(1, &_VBO);
	glBindVertexArray(_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, _VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	u32 vertexShader = createShader(_config.vertexShader, GL_VERTEX_SHADER);
	u32 fragmentShader = createShader(_config.fragmentShader, GL_FRAGMENT_SHADER);

	_shaderProgram = glCreateProgram();
	glAttachShader(_shaderProgram, vertexShader);
	glAttachShader(_shaderProgram, fragmentShader);
	glLinkProgram(_shaderProgram);

	i32 success;
	char infoLog[512];
	glGetProgramiv(_shaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(_shaderProgram, 512, NULL, infoLog);
		Log::Error("Vertex shader compilation failed: ", infoLog);
	}

	glUseProgram(_shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*)0);
	glEnableVertexAttribArray(0);  
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_VBO);
    glDeleteProgram(_shaderProgram);
}

void Renderer::newFrame()
{
	glfwSwapBuffers(_window);
}

void Renderer::draw()
{
	glClearColor(0.5f, 0.0f, 0.7f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(_shaderProgram);
	glBindVertexArray(_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
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

u32 createShader(const char* fileName, GLenum type)
{
	const char* shaderSrc = readFile(fileName);

	u32 shader;
	shader = glCreateShader(type);

	glShaderSource(shader, 1, &shaderSrc, nullptr);
	glCompileShader(shader);

	i32 success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		Log::Error("Vertex shader compilation failed: ", infoLog);
	}

	return shader;
}

char* readFile(const char *fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Cannot open file");

	unsigned int fileSize = (unsigned int)file.tellg();
	char *buffer = (char*)malloc(fileSize);

	file.seekg(0);
	file.read(buffer, fileSize);

	file.close();
	return buffer;
}

static void resizeCallback(GLFWwindow *window, i32 width, i32 height)
{
	ddls::ignore(window);
	glViewport(0, 0, width, height);
}

} // namespace ddls

