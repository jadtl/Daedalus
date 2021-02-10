#include "Shell.h"

#include <cassert>
#include <array>
#include <iostream>
#include <string>
#include <sstream>
#include <set>

#include "Engine.h"

Shell::Shell(Engine &engine) : engine_(engine), settings_(engine.settings()), context_(), engine_tick_(1.0f / settings_.ticks_per_second), engine_time_(engine_tick_) {

    // require generic WSI extensions
    instance_extensions_.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    device_extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // require "standard" validation layers
    if (settings_.validate) {
        
        instance_layers_.push_back("VK_LAYER_KHRONOS_validation");
        instance_extensions_.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        
    }
    
}

void Shell::log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, const char* message) const {

    std::ostream &stream = (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? std::cerr : std::cout;
    stream << message << "\n";
    
}

void Shell::initialize_vulkan() {
    
    initialize_instance();
    initialize_debug_messenger();
    initialize_physical_device();
    
}

void Shell::cleanup_vulkan() {
    
    if (settings_.validate) destroy_debug_utils_messenger(context_.instance, context_.debug_messenger, nullptr);
    
    vkDestroyInstance(context_.instance, nullptr);
    
}

VkResult Shell::create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    
        if (func != nullptr) {
            
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            
        } else {
            
            return VK_ERROR_EXTENSION_NOT_PRESENT;
            
        }
    
}

void Shell::destroy_debug_utils_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    
    if (func != nullptr) {
        
        func(instance, debugMessenger, pAllocator);
        
    }
    
}

void Shell::assert_all_instance_layers() const {
    
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    
    std::set<std::string> layer_names;
    for (const auto &layer : layers) layer_names.insert(layer.layerName);
    
    for (const auto &name : instance_layers_) {
        
        if (layer_names.find(name) == layer_names.end()) {
            
            std::stringstream stream;
            stream << "Instance layer " << name << " is missing!";
            throw std::runtime_error(stream.str());
            
        }
        
    }
    
}

void Shell::assert_all_instance_extensions() const {
    
    std::vector<VkExtensionProperties> extensions;
    uint32_t count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    extensions.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    
    std::set<std::string> extensions_names;
    for (const auto &extension : extensions) extensions_names.insert(extension.extensionName);
    
    for (auto &layer : instance_layers_) {
        
        uint32_t count = 0;
        vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);
        extensions.resize(count);
        vkEnumerateInstanceExtensionProperties(layer, &count, extensions.data());
        
        for (const auto &extension : extensions) extensions_names.insert(extension.extensionName);
        
    }
    
    for (const auto &name : instance_extensions_) {
        
        if (extensions_names.find(name) == extensions_names.end()) {
            
            std::stringstream stream;
            stream << "Instance extension " << name << " is missing!";
            throw std::runtime_error(stream.str());
            
        }
        
    }
    
}

void Shell::populate_debug_messenger_info(VkDebugUtilsMessengerCreateInfoEXT &debug_messenger_info) {
    
    debug_messenger_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    if (settings_.validate_verbose) {
        debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    } else {
        debug_messenger_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    debug_messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_messenger_info.pfnUserCallback = debug_callback;
    debug_messenger_info.pUserData = nullptr;
    
}

void Shell::initialize_instance() {
    
    assert_all_instance_layers();
    assert_all_instance_extensions();
    
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
    instance_info.enabledLayerCount = static_cast<uint32_t>(instance_layers_.size());
    instance_info.ppEnabledLayerNames = instance_layers_.data();
    
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    if (settings_.validate) {
        populate_debug_messenger_info(debug_messenger_info);
        instance_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debug_messenger_info;
    }
    
    instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_.size());
    instance_info.ppEnabledExtensionNames = instance_extensions_.data();
    
    if (vkCreateInstance(&instance_info, nullptr, &context_.instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }
    
}

void Shell::initialize_debug_messenger() {
    
    if (!settings_.validate) return;
    
    VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info{};
    populate_debug_messenger_info(debug_messenger_info);
    
    if (create_debug_utils_messenger(context_.instance, &debug_messenger_info, nullptr, &context_.debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
    
}

void Shell::initialize_physical_device() {
    
    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(context_.instance, &physical_device_count, nullptr);
    
    if (physical_device_count == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }
    
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(context_.instance, &physical_device_count, physical_devices.data());
    
    for (const VkPhysicalDevice& physical_device : physical_devices) {
        
        if (is_physical_device_suitable(physical_device)) {
            
            context_.physical_device = physical_device;
            break;
            
        }
        
    }
    
    if (context_.physical_device == VK_NULL_HANDLE) {
        
        throw std::runtime_error("Failed to find a suitable GPU");
        
    }
    
}

bool Shell::is_physical_device_suitable(VkPhysicalDevice physical_device) {
    
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);
    
    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);
    
    return true;
    
}

void Shell::create_context() {
    
    
    
}

void Shell::destroy_context() {
    
    
    
}

void Shell::create_device() {
    
    VkDeviceCreateInfo device_info{};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    
    const std::vector<float> queue_priorities;
    
}

void Shell::create_sync_objects() {
    
    
    
}

void Shell::destroy_sync_objects() {
    
    
    
}

void Shell::create_swapchain() {
    
    
    
}

void Shell::destroy_swapchain() {
    
    
    
}

void Shell::resize_swapchain(uint32_t width_hint, uint32_t height_hint) {
    
    
    
}

void Shell::add_engine_time(float time) {
    
    
    
}

void Shell::acquire_sync_objects() {
    
    
    
}

void Shell::present_sync_objects() {
    
    
    
}

void Shell::fake_present() {
    
    
    
}
