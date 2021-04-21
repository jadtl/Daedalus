#pragma once

#include <MoltenVK/mvk_vulkan.h>

#include "Shell.h"

#include <sys/time.h>

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
    ShellDarwin(Engine &engine, const char* root, void* caMetalLayer);
    ~ShellDarwin();
    
    void frameStarted();
    void frameEnded();
    
protected:
    void* caMetalLayer;
    PosixTimer timer;
    double currentTime;
    double profileStartTime;
    int profilePresentCount;
    
    VkSurfaceKHR initializeSurface(VkInstance instance);
};
