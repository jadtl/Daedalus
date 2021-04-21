#include "ShellDarwin.h"

#include "Engine.h"

#include <mach/mach_time.h>
#include <cassert>
#include <sstream>
#include <dlfcn.h>

PosixTimer::PosixTimer() {
    _tsBase = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    _tsPeriod = (double)timebase.numer / (double)timebase.denom;
}

double PosixTimer::get() {
    return (double)(mach_absolute_time() - _tsBase) * _tsPeriod / 1e9;
}

ShellDarwin::ShellDarwin(Engine& engine, const char* root, void* caMetalLayer) :
    Shell(engine, root), caMetalLayer(caMetalLayer), timer(PosixTimer()), currentTime(timer.get()) {}

ShellDarwin::~ShellDarwin() {
    vkDestroySurfaceKHR(context().instance, context().surface, nullptr);
}

VkSurfaceKHR ShellDarwin::initializeSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    
    VkMetalSurfaceCreateInfoEXT surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.pLayer = caMetalLayer;
    
    (vkCreateMetalSurfaceEXT(instance, &surfaceInfo, nullptr, &surface) == VK_SUCCESS) ?
    log(LOG_INFO, "Metal surface successfully created") : log(LOG_ERROR, "Metal surface failed to create!");
    
    return surface;
}

void ShellDarwin::updateAndRender() {
    engine_.acquireBackBuffer();
    
    engine_.onTick();
    engine_.onFrame();
    
    engine_.presentBackBuffer();
    
    
}

void ShellDarwin::frameStarted() { currentTime = timer.get(); }

void ShellDarwin::frameEnded() {
    if (currentTime - profileStartTime >= 5.0) {
        const double fps = profilePresentCount / (currentTime - profileStartTime);
        
        std::stringstream ss;
        ss << profilePresentCount << " presents in " << currentTime - profileStartTime
        << " seconds " << "(FPS: " << fps << ")"; log(LOG_INFO, ss.str().c_str());

        profileStartTime = currentTime;
        profilePresentCount = 0;
    }
}

void ShellDarwin::run(void *caMetalLayer) {
    this->caMetalLayer = caMetalLayer;
    
    initializeVulkan();
    initializeContext();
    initializeSwapchain();
}
