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
        
        uint32_t image_index;
        
        VkSemaphore acquire_semaphore;
        VkSemaphore render_semaphore;
        
        VkFence present_fence;
        
    };
    
    struct Context {
        
        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        
        VkPhysicalDevice physical_device;
        uint32_t engine_queue_family;
        uint32_t present_queue_family;
        
        VkDevice device;
        VkQueue engine_queue;
        VkQueue present_queue;
        
        std::queue<SyncObjects> back_buffers;
        
        VkSurfaceKHR surface;
        VkSurfaceFormatKHR format;
        
        VkSwapchainKHR swapchain;
        VkExtent2D extent;
        
        SyncObjects acquired_sync_objects;
        
    };
    
    const Context &context() const { return context_; }
    
    virtual void log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, const char *message) const;
    
    virtual void run() = 0;
    virtual void quit() = 0;
    
protected:
    
    Shell(Engine &engine);
    
    void initialize_vulkan();
    void cleanup_vulkan();
    
    void create_context();
    void destroy_context();
    
    void resize_swapchain(uint32_t width_hint, uint32_t height_hint);
    
    void add_engine_time(float time);
    
    void acquire_sync_objects();
    void present_sync_objects();
    
    Engine &engine_;
    const Engine::Settings &settings_;
    
    std::vector<const char *> instance_layers_;
    std::vector<const char *> instance_extensions_;

    std::vector<const char *> device_extensions_;
    
private:
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                         VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                         const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                         void* user_data) {
        
        message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ?
        std::cerr : std::cout << callback_data->pMessage << "\n";
        
        return VK_FALSE;
    }
    
    VkResult create_debug_utils_messenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,                                   const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger);
    void destroy_debug_utils_messenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);
    
    void assert_all_instance_layers() const;
    void assert_all_instance_extensions() const;
    
    // called by initialize_vulkan
    virtual PFN_vkGetInstanceProcAddr load_vulkan() = 0;
    virtual bool can_present(VkPhysicalDevice physical_device, uint32_t queue_family) = 0;
    void populate_debug_messenger_info(VkDebugUtilsMessengerCreateInfoEXT& debug_messengerx_info);
    void initialize_instance();
    void initialize_debug_messenger();
    void initialize_physical_device();
    bool is_physical_device_suitable(VkPhysicalDevice physical_device);
    uint32_t findQueueFamilies(VkPhysicalDevice physical_device);

    // called by create_context
    void create_device();
    void create_sync_objects();
    void destroy_sync_objects();
    virtual VkSurfaceKHR create_surface(VkInstance instance) = 0;
    void create_swapchain();
    void destroy_swapchain();

    void fake_present();

    Context context_;

    const float engine_tick_;
    float engine_time_;
    
};
