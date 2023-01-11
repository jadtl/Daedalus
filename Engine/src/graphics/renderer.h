#pragma once

#include <core/defines.h>
#include <core/types.h>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include <optional>

namespace ddls {
/**
 * @brief The Vulkan renderer
 * 
 */
class DDLS_API Renderer
{
public:
    Renderer(GLFWwindow *window, const char* appName, const char* engineName);
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

    std::vector<const char*> getRequiredExtensions(vk::InstanceCreateInfo& createInfo);
    void enumerateAvailableExtensions();
    bool checkValidationLayersSupport();
    struct QueueFamilyIndices {
        std::optional<u32> graphicsFamily;
        bool isComplete()
        {
            return graphicsFamily.has_value();
        }
    };
};
}