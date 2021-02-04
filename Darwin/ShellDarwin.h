#pragma once

#include <sys/time.h>

#include <MoltenVK/mvk_vulkan.h>

#include "Shell.h"

class PosixTimer {
    
public:
    
    double get();
    PosixTimer();
    
protected:
    
    uint64_t _tsBase;
    double _tsPeriod;
    
};

class ShellDarwin : public Shell {
    
public:
    
    ShellDarwin(Engine &engine);
    ~ShellDarwin();
    
    void run(void* view);
    void update_and_draw();
    
    void run() { run(nullptr); }
    void quit() { }
    
protected:
    
    void* _caMetalLayer;
    PosixTimer _timer;
    double _current_time;
    double _profile_start_time;
    int _profile_present_count;
    
    PFN_vkGetInstanceProcAddr load_vulkan();
    bool can_present(VkPhysicalDevice physical_device, uint32_t queue_family);

    VkSurfaceKHR create_surface(VkInstance instance);
    
};
