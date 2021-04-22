#pragma once

#include <MoltenVK/mvk_vulkan.h>

#include "Platform.h"

#include <sys/time.h>

class PosixTimer {
public:
    double get();
    PosixTimer();
    
protected:
    uint64_t _tsBase;
    double _tsPeriod;
};

class PlatformDarwin : public Platform {
public:
    PlatformDarwin(Engine &engine, const char* root, void* caMetalLayer);
    ~PlatformDarwin();
    
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
