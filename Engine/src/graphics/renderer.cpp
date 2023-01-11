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

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    Log::Assert(deviceCount, 
        "No physical device was found!");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

    // Find a suitable physical device
    for (const auto& physicalDevice: physicalDevices)
    {
        if (isDeviceSuitable(physicalDevice))
        {
            _physicalDevice = physicalDevice;
            break;
        }
    }

    Log::Assert(_physicalDevice != VK_NULL_HANDLE, 
        "No suitable physical device was found!");

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

    Log::Info("Picking physical device ", physicalDeviceProperties.deviceName);

    u32 extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());
    Log::Info("Device extensions");
    for (const auto& extension : extensions)
        Log::Info('\t', extension.extensionName);

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
#ifdef DDLS_PLATFORM_MACOS
    std::vector<const char*> enabledExtensions;
    enabledExtensions.push_back("VK_KHR_portability_subset");
    deviceCreateInfo.enabledExtensionCount = (u32)enabledExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
#endif
    if (_enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    Log::Assert(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) == VK_SUCCESS,
        "Failed to create device!");

    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
}

Renderer::~Renderer()
{
    vkDestroyDevice(_device, nullptr);

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

bool Renderer::isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    Log::Info("Checking if device ", physicalDeviceProperties.deviceName, " is suitable");
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    Renderer::QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    // Select a physical device that supports graphics commands
    if (!indices.isComplete()) return false;

    Log::Info("Physical device ", physicalDeviceProperties.deviceName, " meets the requirements");
    return true;
}

Renderer::QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Finding a queue that supports graphics commands
    Renderer::QueueFamilyIndices indices;
    int i = 0;
    for (const auto& queueFamily: queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}
}