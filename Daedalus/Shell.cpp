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
        instance_extensions_.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    
}

void Shell::log(LogPriority priority, const char* message) const {
    
    std::ostream &stream = (priority >= LOG_ERROR) ? std::cerr : std::cout;
    stream << message << "\n";
    
}

void Shell::initialize_vulkan() {
    
    init_instance();
    init_debug_report();
    init_physical_device();
    
}

void Shell::cleanup_vulkan() {
    
    
    
}

bool Shell::debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t message_code, const char *layer_prefix, const char *message) {
    
    
    
    return false;
}

void Shell::assert_all_instance_layers() const {
    
    std::vector<VkLayerProperties> layers;
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    layers.resize(count);
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

bool Shell::has_all_device_extensions(VkPhysicalDevice physical_device) const {
    
    
    
    return true;
}

void Shell::init_instance() {
    
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
    instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_.size());
    instance_info.ppEnabledExtensionNames = instance_extensions_.data();
    
    if (vkCreateInstance(&instance_info, nullptr, &context_.instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }
    
}

void Shell::init_debug_report() {
    
    
    
}

void Shell::init_physical_device() {
    
    
    
}

void Shell::create_context() {
    
    
    
}

void Shell::destroy_context() {
    
    
    
}

void Shell::create_device() {
    
    
    
}

void Shell::create_back_buffers() {
    
    
    
}

void Shell::destroy_back_buffers() {
    
    
    
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
