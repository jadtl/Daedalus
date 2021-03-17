#pragma once

#include <vector>
#include <string>

#include <MoltenVK/mvk_vulkan.h>

#include "Types.h"

class Engine {
public:
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    
    Engine(const std::vector<std::string> &args, void *caMetalLayer);
    ~Engine();
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    
    bool isInitialized{false};
    int frameNumber{0};
    
    struct Settings {
        std::string engineName = "Daedalus";
        std::string applicationName = "The Architect";
        
        VkExtent2D windowExtent{1024, 800};
        
        bool validate{false};
        bool verbose{false};
    };
    Settings settings;
    
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
    
    void run();
    
    void update();
    void render();
    
private:
    void* caMetalLayer;
    
    std::vector<const char *> instanceLayers;
    std::vector<const char *> instanceExtensions;
    
    void initialize();
    void terminate();
    
    void initializeSwapchain();
};
