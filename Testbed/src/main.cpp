#include <daedalus.h>

#include <GLFW/glfw3.h>

#include <memory>

const char *appName = "Daedalus";

const char *engineName = "Daedalus";

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(800, 600, appName, nullptr, nullptr);

    std::unique_ptr<ddls::Renderer> renderer = std::make_unique<ddls::Renderer>(window, appName, engineName);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        renderer->render();
    }

    vkDeviceWaitIdle(renderer->device());

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}