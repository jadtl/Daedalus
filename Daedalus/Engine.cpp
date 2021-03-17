#include "Engine.h"

#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"

#include <algorithm>

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

Engine::Engine(const std::vector<std::string> &args) {
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
    vkb::InstanceBuilder builder;
    
    auto instanceBuilder = builder.set_engine_name(settings.engineName.c_str())
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName.c_str())
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0);
    
    if (settings.validate) instanceBuilder.request_validation_layers(true).use_default_debug_messenger();
    if (settings.verbose) instanceBuilder.add_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT);
    
    std::for_each(instanceLayers.begin(), instanceLayers.end(), [&instanceBuilder](char* instanceLayer) {
        instanceBuilder.enable_layer(instanceLayer); });
    std::for_each(instanceExtensions.begin(), instanceExtensions.end(), [&instanceBuilder](char* instanceLayer) {
        instanceBuilder.enable_extension(instanceLayer); });
    
    vkb::Instance vkbInstance = instanceBuilder.build().value();
    
    instance = vkbInstance.instance;
    debugMessenger = vkbInstance.debug_messenger;
    
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

void Engine::run(void *caMetalLayer) {
    this->caMetalLayer = caMetalLayer;
    // Create context
    // Create/Resize swapchain
}

void Engine::update() {
    
}

void Engine::render() {
    
}

void Engine::onKey(Key key) {
    
}
