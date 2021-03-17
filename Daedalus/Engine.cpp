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
    
    if (std::find(args.begin(), args.end(), "-v") != args.end()) {
        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    // Darwin specific (MoltenVK/mvk_vulkan.h)
    instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
}

Engine::~Engine() {
    // Destroy context
    cleanUp();
}

void Engine::initialize() {
    isInitialized = true;
}

void Engine::cleanUp() {
    
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
