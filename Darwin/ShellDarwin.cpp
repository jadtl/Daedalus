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
    _current_time = _timer.get();
    _profile_start_time = _current_time;
    _profile_present_count = 0;

    instance_extensions_.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
    
    initialize_vulkan();
    
}

ShellDarwin::~ShellDarwin() {
    
    destroy_context();
    cleanup_vulkan();
    
}

PFN_vkGetInstanceProcAddr ShellDarwin::load_vulkan()
{
    
    return vkGetInstanceProcAddr;
    
}

bool ShellDarwin::can_present(VkPhysicalDevice physical_device, uint32_t queue_family)
{
    
    return true;
    
}

VkSurfaceKHR ShellDarwin::create_surface(VkInstance instance) {
    
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

void ShellDarwin::update_and_draw() {

    acquire_sync_objects();

    double t = _timer.get();
    add_engine_time(static_cast<float>(t - _current_time));

    present_sync_objects();

    _current_time = t;

    _profile_present_count++;
    
    if (_current_time - _profile_start_time >= 5.0) {
        
        const double fps = _profile_present_count / (_current_time - _profile_start_time);
        std::stringstream ss;
        ss << _profile_present_count << " presents in " <<
        _current_time - _profile_start_time << " seconds " <<
        "(FPS: " << fps << ")";
        log(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT, ss.str().c_str());

        _profile_start_time = _current_time;
        _profile_present_count = 0;
        
    }
    
}

void ShellDarwin::run(void* caMetalLayer) {
    
    _caMetalLayer = caMetalLayer;
    create_context();
    resize_swapchain(settings_.initial_extent.width, settings_.initial_extent.height);
    
}

