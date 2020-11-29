#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>

namespace ddls
{
void createSurface(VkInstance instance,
                   GLFWwindow *window,
                   VkSurfaceKHR *surface);
}
