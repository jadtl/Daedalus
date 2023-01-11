#include "graphics/renderer.h"

#include <core/log.h>
#include "renderer.h"

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
    Log::Assert(checkValidationLayersSupport() || !_enableValidationLayers,
        "Validation layers were requested but not supported!");

    enumerateAvailableExtensions();

    vk::ApplicationInfo appInfo(appName, 1, engineName, 1, VK_API_VERSION_1_0);

    vk::InstanceCreateInfo createInfo({}, &appInfo);
    std::vector<const char*> requiredExtensions = getRequiredExtensions(createInfo);
    createInfo.setPEnabledExtensionNames(requiredExtensions);
#ifdef DDLS_DEBUG
    vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
    debugInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
    );
    debugInfo.setMessageType(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
    );
    debugInfo.setPfnUserCallback(debugCallback);
    createInfo.setPNext(&debugInfo);
    createInfo.setPEnabledLayerNames(_validationLayers);
#endif

    _instance = std::make_unique<vk::raii::Instance>(_context, createInfo);

    _physicalDevices = std::make_unique<vk::raii::PhysicalDevices>(*_instance);
    
    b8 suitableDeviceFound = false;
    for (u32 i = 0; i < (*_physicalDevices).size(); i++)
    {
        if (isDeviceSuitable((*_physicalDevices)[i]))
        {
            _physicalDeviceIndex = i;
            suitableDeviceFound = true;
            break;
        }
    }

    Log::Assert(suitableDeviceFound, "No suitable physical device was found!");

    Log::Info("Picking physical device ", (*_physicalDevices)[_physicalDeviceIndex].getProperties().deviceName);
}

std::vector<const char*> Renderer::getRequiredExtensions(vk::InstanceCreateInfo& createInfo)
{
    // Get required extensions for windowing
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = 
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions;
    for (u32 i = 0; i < glfwExtensionCount; i++)
        requiredExtensions.push_back(glfwExtensions[i]);

    // Add portability enumeration extension when on Apple platform
#ifdef DDLS_PLATFORM_MACOS
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.setFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    // Add debug utils if validation layers are requested
    if (_enableValidationLayers)
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return requiredExtensions;
}
void Renderer::enumerateAvailableExtensions()
{
    Log::Info("Available extensions: ");
    for (const auto& extension : _context.enumerateInstanceExtensionProperties())
        Log::Info('\t', extension.extensionName);
}
bool Renderer::checkValidationLayersSupport()
{
    for (std::string layerName: _validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties: 
            _context.enumerateInstanceLayerProperties())
        {
            if (layerName == layerProperties.layerName)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) return false;
    }

    return true;
}

bool Renderer::isDeviceSuitable(vk::raii::PhysicalDevice physicalDevice)
{
    vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();
    Log::Info("Checking if device ", physicalDeviceProperties.deviceName, " is suitable");
    vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();

    Renderer::QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    // Select a physical device that supports graphics commands
    if (!indices.isComplete()) return false;

    Log::Info("Physical device ", physicalDeviceProperties.deviceName, " meets the requirements");
    return true;
}

Renderer::QueueFamilyIndices Renderer::findQueueFamilies(vk::raii::PhysicalDevice physicalDevice)
{
    // Finding a queue that supports graphics commands
    Renderer::QueueFamilyIndices indices;
    int i = 0;
    for (const auto& queueFamily: physicalDevice.getQueueFamilyProperties())
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}
}