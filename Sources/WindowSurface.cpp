#include <WindowSurface.hpp>

namespace ddls
{
void createSurface(VkInstance instance, GLFWwindow *window, VkSurfaceKHR *surface)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create window surface!");
    }
}
}
