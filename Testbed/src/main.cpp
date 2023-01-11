#include <daedalus.h>

#include <GLFW/glfw3.h>

#include <memory>

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Daedalus", nullptr, nullptr);

    std::unique_ptr<ddls::Renderer> renderer = std::make_unique<ddls::Renderer>(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}