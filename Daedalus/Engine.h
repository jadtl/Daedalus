#pragma once

#include <vector>
#include <string>

#include "Types.h"

class Engine {
public:
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    ~Engine();
    
    Engine(const std::vector<std::string> &args);
    
    bool isInitialized{false};
    int frameNumber{0};
    VkExtent2D windowExtent{1700, 900};
    
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
    
    void* caMetalLayer;
    
    void initialize();
    void cleanup();
    
    void render();
    void run();
    
private:
};
