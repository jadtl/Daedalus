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
    void updateAndDraw();
    
    void run() { run(nullptr); }
    void quit() { }
    
protected:
    
    void* _caMetalLayer;
    PosixTimer _timer;
    double _currentTime;
    double _profileStartTime;
    int _profilePresentCount;
    
    PFN_vkGetInstanceProcAddr loadVulkan();
    bool canPresent(VkPhysicalDevice physicalDevice, uint32_t queueFamily);

    VkSurfaceKHR createSurface(VkInstance instance);
    
};
