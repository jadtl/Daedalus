#pragma once

#include <vector>
#include <string>

#include <MoltenVK/mvk_vulkan.h>

#include "Types.h"

class Engine {
public:
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    ~Engine();
    
    Engine(const std::vector<std::string> &args);
    
    bool isInitialized{false};
    int frameNumber{0};
    VkExtent2D windowExtent{1024, 800};
    
    enum Key {
        KEY_A,
        KEY_S,
        KEY_D,
        KEY_Q,
        KEY_W,
        KEY_E,
        KEY_SPACE,
    };
    
    void onKey(Key key);
    
    void initialize();
    void cleanUp();
    
    void run(void* caMetalLayer);
    void update();
    void render();
    
private:
    void* caMetalLayer;
    
    std::vector<const char *> instanceLayers;
    std::vector<const char *> instanceExtensions;
};
