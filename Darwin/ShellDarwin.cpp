#include "ShellDarwin.h"

#include <mach/mach_time.h>
#include <cassert>
#include <sstream>
#include <dlfcn.h>

#include "Engine.h"

PosixTimer::PosixTimer() {
    
    _tsBase = mach_absolute_time();
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    _tsPeriod = (double)timebase.numer / (double)timebase.denom;
    
}

double PosixTimer::get() {
    
    return (double)(mach_absolute_time() - _tsBase) * _tsPeriod / 1e9;
    
}

ShellDarwin::ShellDarwin(Engine& engine) : Shell(engine) {
    
    _timer = PosixTimer();
    _currentTime = _timer.get();
    _profileStartTime = _currentTime;
    _profilePresentCount = 0;

    instanceExtensions_.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    
    initializeVulkan();
    
}

ShellDarwin::~ShellDarwin() {
    
    destroyContext();
    cleanupVulkan();
    
}

PFN_vkGetInstanceProcAddr ShellDarwin::loadVulkan()
{
    
    return vkGetInstanceProcAddr;
    
}

bool ShellDarwin::canPresent(VkPhysicalDevice physical_device, uint32_t queue_family)
{
    
    return true;
    
}

VkSurfaceKHR ShellDarwin::createSurface(VkInstance instance) {
    
    VkSurfaceKHR surface;

    VkMetalSurfaceCreateInfoEXT surface_info;
    surface_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surface_info.pNext = NULL;
    surface_info.flags = 0;
    surface_info.pLayer = _caMetalLayer;
    
    if (vkCreateMetalSurfaceEXT(instance, &surface_info, NULL, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Metal surface!");
    };

    return surface;
    
}

void ShellDarwin::updateAndDraw() {

    acquireSyncObjects();

    double t = _timer.get();
    addEngineTime(static_cast<float>(t - _currentTime));

    presentSyncObjects();

    _currentTime = t;

    _profilePresentCount++;
    
    if (_currentTime - _profileStartTime >= 5.0) {
        
        const double fps = _profilePresentCount / (_currentTime - _profileStartTime);
        std::stringstream ss;
        ss << _profilePresentCount << " presents in " <<
        _currentTime - _profileStartTime << " seconds " <<
        "(FPS: " << fps << ")";
        log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, ss.str().c_str());

        _profileStartTime = _currentTime;
        _profilePresentCount = 0;
        
    }
    
}

void ShellDarwin::run(void* caMetalLayer) {
    
    _caMetalLayer = caMetalLayer;
    createContext();
    resizeSwapchain(settings_.initialExtent.width, settings_.initialExtent.height);
    
}

