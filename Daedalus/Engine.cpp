#include "Engine.h"

#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"

#include <algorithm>
#include <iostream>

// We want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state
#define VK_CHECK(x)                                                      \
    do                                                                   \
    {                                                                    \
        VkResult error = x;                                              \
        if (error)                                                       \
        {                                                                \
            std::cout <<"Detected Vulkan error: " << error << std::endl; \
            abort();                                                     \
        }                                                                \
    } while (0)

Engine::Engine(const std::vector<std::string> &args, void *caMetalLayer) : caMetalLayer(caMetalLayer) {
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    
    if (std::find(args.begin(), args.end(), "-validate") != args.end()) { settings.validate = true; }
    if (std::find(args.begin(), args.end(), "-verbose") != args.end()) { settings.validate = true; settings.verbose = true; }
    
    // Darwin specific (MoltenVK/mvk_vulkan.h)
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    
    initialize();
}

Engine::~Engine() {
    // Destroy context
    terminate();
}

void Engine::initialize() {
    // Instance and debug messenger creation
    vkb::InstanceBuilder builder;
    
    auto instanceBuilder = builder.set_engine_name(settings.engineName.c_str())
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName.c_str())
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0);
    
    if (settings.validate) instanceBuilder.request_validation_layers(true).use_default_debug_messenger();
    if (settings.verbose) instanceBuilder.add_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT);
    
    std::for_each(instanceLayers.begin(), instanceLayers.end(), [&instanceBuilder](const char* instanceLayer) {
        instanceBuilder.enable_layer(instanceLayer); });
    std::for_each(instanceExtensions.begin(), instanceExtensions.end(), [&instanceBuilder](const char* instanceLayer) {
        instanceBuilder.enable_extension(instanceLayer); });
    
    vkb::Instance vkbInstance = instanceBuilder.build().value();
    
    this->instance = vkbInstance.instance;
    this->debugMessenger = vkbInstance.debug_messenger;
    
    // Surface creation
    VkMetalSurfaceCreateInfoEXT surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pNext = VK_NULL_HANDLE;
    surfaceInfo.flags = 0;
    surfaceInfo.pLayer = caMetalLayer;
    VK_CHECK(vkCreateMetalSurfaceEXT(this->instance, &surfaceInfo, VK_NULL_HANDLE, &this->surface));
    
    // Physical device creation
    vkb::PhysicalDeviceSelector physicalDeviceSelector{vkbInstance};
    vkb::PhysicalDevice physicalDevice = physicalDeviceSelector
        .set_minimum_version(1, 1)
        .set_surface(this->surface)
        .add_desired_extension("VK_KHR_portability_subset")
        .select()
        .value();
    
    // Device creation
    vkb::DeviceBuilder deviceBuilder{physicalDevice};
    vkb::Device vkbDevice = deviceBuilder.build().value();
    
    this->device = vkbDevice.device;
    this->physicalDevice = physicalDevice.physical_device;
    
    initializeSwapchain();
    
    isInitialized = true;
}

void Engine::terminate() {
    if (isInitialized) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        std::for_each(swapchainImageViews.begin(), swapchainImageViews.end(), [device = device](VkImageView imageView) {
            vkDestroyImageView(device, imageView, nullptr); });
        
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        vkDestroyInstance(instance, nullptr);
    }
}

void Engine::run() {
    // Create context
    // Create/Resize swapchain
}

void Engine::update() {
    
}

void Engine::render() {
    
}

void Engine::onKey(Key key) {
    
}

void Engine::initializeSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{physicalDevice, device, surface};
    
    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(settings.windowExtent.width, settings.windowExtent.height)
        .build()
        .value();
    
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
    
    swapchainImageFormat = vkbSwapchain.image_format;
}
