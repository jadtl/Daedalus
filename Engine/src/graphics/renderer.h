#pragma once

#include <core/defines.h>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

namespace ddls {
/**
 * @brief The Vulkan renderer
 * 
 */
class DDLS_API Renderer
{
public:
    Renderer(GLFWwindow *window);
private:
    GLFWwindow *window;

    vk::raii::Context _context;
    vk::Instance _instance;

    const std::vector<const char*> _validationLayers = 
        {"VK_LAYER_KHRONOS_validation"};
#ifdef DDLS_DEBUG
    const bool _enableValidationLayers = true;
#else
    const bool _enableValidationLayers = false;
#endif
};
}