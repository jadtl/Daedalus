#include "graphics/renderer.h"

#include <core/log.h>

namespace ddls {
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    ignore(messageSeverity, messageType, pUserData);

    Log::Out(Log::Styles::Default, Log::Colours::LightBlue, "VALIDATION", pCallbackData->pMessage);

    return VK_FALSE;
}

Renderer::Renderer(GLFWwindow *window, const char* appName, const char* engineName)
{
    Log::Assert(checkValidationLayerSupport(_validationLayers) || !_enableValidationLayers, 
        "Validations layers requested but not supported!");

    // Fill application info
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = appName;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = engineName;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // Fill instance create info
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    std::vector<const char*> requiredExtensions = 
        getRequiredExtensions(&instanceCreateInfo, _enableValidationLayers);
    instanceCreateInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if (_enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        fillDebugMessengerCreateInfo(debugCreateInfo, debugCallback);
        instanceCreateInfo.pNext = &debugCreateInfo;
        instanceCreateInfo.enabledLayerCount = (uint32_t)_validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }

    enumerateAvailableExtensions();

    Log::Assert(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance) == VK_SUCCESS, 
        "Instance creation failed!");

#ifdef DDLS_DEBUG

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    fillDebugMessengerCreateInfo(debugCreateInfo, debugCallback);

    auto createFunction = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    Log::Assert(createFunction, 
        "Failed to get vkCreateDebugUtilsMessengerEXT function!");
    Log::Assert(createFunction(_instance, &debugCreateInfo, nullptr, &_debugMessenger) == VK_SUCCESS,
        "Failed to create debug messenger!");
#endif
}

Renderer::~Renderer()
{
#ifdef DDLS_DEBUG
    auto destroyFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    destroyFunction(_instance, _debugMessenger, nullptr);
#endif

    vkDestroyInstance(_instance, nullptr);
}

std::vector<const char*> Renderer::getRequiredExtensions(VkInstanceCreateInfo* instanceCreateInfo, bool enableValidationLayers)
{
    // Get required extensions for windowing
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = 
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions;
    for (u32 i = 0; i < glfwExtensionCount; i++)
        requiredExtensions.push_back(glfwExtensions[i]);

    // Add portability enumeration extension when on Apple platform
#ifdef __APPLE__
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceCreateInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    // Add debug utils if validation layers are requested
    if (enableValidationLayers)
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return requiredExtensions;
}

void Renderer::enumerateAvailableExtensions()
{
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    Log::Info("Available extensions");
    for (const auto& extension : extensions)
        Log::Info('\t', extension.extensionName);
}

bool Renderer::checkValidationLayerSupport(std::vector<const char*> validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName: validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties: availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) return false;
    }

    return true;
}

void Renderer::fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo, PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.flags = 0;
    createInfo.pUserData = nullptr;
}

}