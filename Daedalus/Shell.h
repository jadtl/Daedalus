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
    
    struct BackBuffer {
        
        uint32_t image_index;
        
        VkSemaphore acquire_semaphore;
        VkSemaphore render_semaphore;
        
        VkFence present_fence;
        
    };
    
    struct Context {
        
        VkInstance instance;
        VkDebugReportCallbackEXT debug_report;
        
        VkPhysicalDevice physical_device;
        uint32_t engine_queue_family;
        uint32_t present_queue_family;
        
        VkDevice device;
        VkQueue engine_queue;
        VkQueue present_queue;
        
        std::queue<BackBuffer> back_buffers;
        
        VkSurfaceKHR surface;
        VkSurfaceFormatKHR format;
        
        VkSwapchainKHR swapchain;
        VkExtent2D extent;
        
        BackBuffer acquired_back_buffer;
        
    };
    
    const Context &context() const { return context_; }
    
    enum LogPriority {
        
        LOG_DEBUG,
        LOG_INFO,
        LOG_WARNING,
        LOG_ERROR
        
    };
    
    virtual void log(LogPriority priority, const char *message) const;
    
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
    
    void acquire_back_buffer();
    void present_back_buffer();
    
    Engine &engine_;
    const Engine::Settings &settings_;
    
    std::vector<const char *> instance_layers_;
    std::vector<const char *> instance_extensions_;

    std::vector<const char *> device_extensions_;
    
private:
    
    bool debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t message_code, const char *layer_prefix, const char *message);
    
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object, size_t location, int32_t message_code, const char *layer_prefix, const char *message, void *user_data) {
        
        Shell *shell = reinterpret_cast<Shell *>(user_data);
        return shell->debug_report_callback(flags, object_type, object, location, message_code, layer_prefix, message);
        
    }
    
    void assert_all_instance_layers() const;
    void assert_all_instance_extensions() const;

    bool has_all_device_layers(VkPhysicalDevice physical_device) const;
    bool has_all_device_extensions(VkPhysicalDevice physical_device) const;

    // called by init_vk
    virtual PFN_vkGetInstanceProcAddr load_vulkan() = 0;
    virtual bool can_present(VkPhysicalDevice physical_device, uint32_t queue_family) = 0;
    void init_instance();
    void init_debug_report();
    void init_physical_device();

    // called by create_context
    void create_device();
    void create_back_buffers();
    void destroy_back_buffers();
    virtual VkSurfaceKHR create_surface(VkInstance instance) = 0;
    void create_swapchain();
    void destroy_swapchain();

    void fake_present();

    Context context_;

    const float engine_tick_;
    float engine_time_;
    
};
