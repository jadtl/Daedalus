#include <ValidationLayers.hpp>

#include <cstring>

namespace ddls
{
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* createInfo, const VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, createInfo, allocator, debugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* allocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, allocator);
    }
}

void populateDebugMessengerCreateInfo
 (VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
}

void setupDebugMessenger(VkInstance instance,
                               VkDebugUtilsMessengerEXT* debugMessenger)
{
    if (!enableValidationLayers) return;
    
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

std::vector<const char*> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> vExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        vExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return vExtensions;
}

bool checkValidationLayerSupport(const std::vector<const char*> validationLayers)
{
    uint32_t nLayerCount;
    vkEnumerateInstanceLayerProperties(&nLayerCount, nullptr);

    std::vector<VkLayerProperties> vAvailableLayers(nLayerCount);
    vkEnumerateInstanceLayerProperties(&nLayerCount, vAvailableLayers.data());

    for (const char* layerName : validationLayers) {
        bool bLayerFound = false;

        for (const auto& layerProperties : vAvailableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                bLayerFound = true;
                break;
            }
        }

        if (!bLayerFound) {
            return false;
        }
    }

    return true;
}
}
