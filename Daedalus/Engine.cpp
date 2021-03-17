#include "Engine.h"

#include "Types.h"
#include "Initializers.h"

#include <algorithm>

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
