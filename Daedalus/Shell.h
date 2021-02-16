#pragma once

#include <queue>
#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.hpp>

#include "Engine.h"

class Engine;

class Shell {
    
public:
    
    Shell(const Shell &shell) = delete;
    Shell &operator = (const Shell &shell) = delete;
    virtual ~Shell() {}
    
    struct SyncObjects {
        
        uint32_t imageIndex;
        
        VkSemaphore acquireSemaphore;
        VkSemaphore renderSemaphore;
        
        VkFence presentFence;
        
    };
    
    struct Context {
        
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        
        VkPhysicalDevice physicalDevice;
        uint32_t engineQueueFamily;
        uint32_t presentQueueFamily;
        
        VkDevice device;
        VkQueue engineQueue;
        VkQueue presentQueue;
        
        std::queue<SyncObjects> syncObjects;
        
        VkSurfaceKHR surface;
        VkSurfaceFormatKHR format;
        
        VkSwapchainKHR swapchain;
        VkExtent2D extent;
        
        SyncObjects acquiredSyncObjects;
        
    };
    
    const Context &context() const { return context_; }
    
    virtual void log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, const char *message) const;
    
    virtual void run() = 0;
    virtual void quit() = 0;
    
protected:
    
    Shell(Engine &engine);
    
    void initializeVulkan();
    void cleanupVulkan();
    
    void createContext();
    void destroyContext();
    
    void resizeSwapchain(uint32_t width_hint, uint32_t height_hint);
    
    void addEngineTime(float time);
    
    void acquireSyncObjects();
    void presentSyncObjects();
    
    Engine &engine_;
    const Engine::Settings &settings_;
    
    std::vector<const char *> instanceLayers_;
    std::vector<const char *> instanceExtensions_;

    std::vector<const char *> deviceExtensions_;
    
private:
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                         VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                         void* pUserData) {
        
        messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ?
        std::cerr : std::cout << pCallbackData -> pMessage << "\n";
        
        return VK_FALSE;
    }
    
    VkResult createDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,                                   const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    
    void assertAllInstanceLayers() const;
    void assertAllInstanceExtensions() const;
    
    // called by initializeVulkan
    virtual PFN_vkGetInstanceProcAddr loadVulkan() = 0;
    virtual bool canPresent(VkPhysicalDevice physical_device, uint32_t queue_family) = 0;
    void populateDebugMessengerInfo(VkDebugUtilsMessengerCreateInfoEXT& debug_messengerx_info);
    void initializeInstance();
    void initializeDebugMessenger();
    void initializePhysicalDevice();
    bool isPhysicalDeviceSuitable(VkPhysicalDevice physical_device);
    uint32_t findQueueFamilies(VkPhysicalDevice physical_device);

    // called by create_context
    void createDevice();
    void createSyncObjects();
    void destroySyncObjects();
    virtual VkSurfaceKHR createSurface(VkInstance instance) = 0;
    void createSwapchain();
    void destroySwapchain();

    void fakePresent();

    Context context_;

    const float engineTick_;
    float engineTime_;
    
};
