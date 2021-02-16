#include "Shell.h"

#include <cassert>
#include <array>
#include <iostream>
#include <string>
#include <sstream>
#include <set>

#include "Engine.h"

Shell::Shell(Engine &engine) : engine_(engine), settings_(engine.settings()), context_(), engineTick_(1.0f / settings_.ticksPerSecond), engineTime_(engineTick_) {

    // require generic WSI extensions
    instanceExtensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    deviceExtensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // require "standard" validation layers
    if (settings_.validate) {
        
        instanceLayers_.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
    }
    
}

void Shell::log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, const char* message) const {

    std::ostream &stream = (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? std::cerr : std::cout;
    stream << message << "\n";
    
}

void Shell::initializeVulkan() {
    
    initializeInstance();
    initializeDebugMessenger();
    initializePhysicalDevice();
    
}

void Shell::cleanupVulkan() {
    
    if (settings_.validate) destroyDebugUtilsMessenger(context_.instance, context_.debugMessenger, nullptr);
    
    vkDestroyInstance(context_.instance, nullptr);
    
}

VkResult Shell::createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    
        if (func != nullptr) {
            
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            
        } else {
            
            return VK_ERROR_EXTENSION_NOT_PRESENT;
            
        }
    
}

void Shell::destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if (func != nullptr) {
        
        func(instance, debugMessenger, pAllocator);
        
    }
    
}

void Shell::assertAllInstanceLayers() const {
    
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    
    std::set<std::string> layer_names;
    for (const auto &layer : layers) layer_names.insert(layer.layerName);
    
    for (const auto &name : instanceLayers_) {
        
        if (layer_names.find(name) == layer_names.end()) {
            
            std::stringstream stream;
            stream << "Instance layer " << name << " is missing!";
            throw std::runtime_error(stream.str());
            
        }
        
    }
    
}

void Shell::assertAllInstanceExtensions() const {
    
    std::vector<VkExtensionProperties> extensions;
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    extensions.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    
    std::set<std::string> extensions_names;
    for (const auto &extension : extensions) extensions_names.insert(extension.extensionName);
    
    for (auto &layer : instanceLayers_) {
        
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
        extensions.resize(count);
        vkEnumerateInstanceExtensionProperties(layer, &count, extensions.data());
        
        for (const auto &extension : extensions) extensions_names.insert(extension.extensionName);
        
    }
    
    for (const auto &name : instanceExtensions_) {
        
        if (extensions_names.find(name) == extensions_names.end()) {
            
            std::stringstream stream;
            stream << "Instance extension " << name << " is missing!";
            throw std::runtime_error(stream.str());
            
        }
        
    }
    
}

void Shell::populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT &debug_messenger_info) {
    
    debug_messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    if (settings_.validateVerbose) {
        debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    } else {
        debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    debug_messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_messenger_info.pfnUserCallback = debugCallback;
    debug_messenger_info.pUserData = nullptr;
    
}

void Shell::initializeInstance() {
    
    assertAllInstanceLayers();
    assertAllInstanceExtensions();
    
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = settings_.name.c_str();
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "Daedalus";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo instance_info{};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &application_info;
    instance_info.enabledLayerCount = static_cast<uint32_t>(instanceLayers_.size());
    instance_info.ppEnabledLayerNames = instanceLayers_.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    if (settings_.validate) {
        populateDebugMessengerInfo(debug_messenger_info);
        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_messenger_info;
    }
    
    instance_info.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions_.size());
    instance_info.ppEnabledExtensionNames = instanceExtensions_.data();
    
    if (vkCreateInstance(&instance_info, nullptr, &context_.instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }
    
}

void Shell::initializeDebugMessenger() {
    
    if (!settings_.validate) return;
    
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    populateDebugMessengerInfo(debug_messenger_info);
    
    if (createDebugUtilsMessenger(context_.instance, &debug_messenger_info, nullptr, &context_.debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
    
}

void Shell::initializePhysicalDevice() {
    
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(context_.instance, &physical_device_count, nullptr);
    
    if (physical_device_count == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(context_.instance, &physical_device_count, physical_devices.data());
    
    for (const VkPhysicalDevice& physical_device : physical_devices) {
        
        if (isPhysicalDeviceSuitable(physical_device)) {
            
            context_.physicalDevice = physical_device;
            break;
            
        }
        
    }
    
    if (context_.physicalDevice == VK_NULL_HANDLE) {
        
        throw std::runtime_error("Failed to find a suitable GPU");
        
    }
    
}

bool Shell::isPhysicalDeviceSuitable(VkPhysicalDevice physical_device) {
    
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    
    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
    
    return true;
    
}

void Shell::createContext() {
    
    
    
}

void Shell::destroyContext() {
    
    
    
}

void Shell::createDevice() {
    
    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    const std::vector<float> queue_priorities;
    
}

void Shell::createSyncObjects() {
    
    
    
}

void Shell::destroySyncObjects() {
    
    
    
}

void Shell::createSwapchain() {
    
    
    
}

void Shell::destroySwapchain() {
    
    
    
}

void Shell::resizeSwapchain(uint32_t width_hint, uint32_t height_hint) {
    
    
    
}

void Shell::addEngineTime(float time) {
    
    
    
}

void Shell::acquireSyncObjects() {
    
    
    
}

void Shell::presentSyncObjects() {
    
    
    
}

void Shell::fakePresent() {
    
    
    
}
