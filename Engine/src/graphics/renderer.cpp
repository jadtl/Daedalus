#include "graphics/renderer.h"

#include <core/log.h>

namespace ddls {
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    Log::Out(Log::Styles::Default, Log::Colours::LightRed, "VALIDATION", pCallbackData->pMessage);

    return VK_FALSE;
}

Renderer::Renderer(GLFWwindow *window)
{
    
}
}