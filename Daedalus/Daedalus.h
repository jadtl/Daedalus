#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "Scene.h"
#include "Engine.h"

class Daedalus : public Engine {
    
public:
    
    Daedalus(const std::vector<std::string> &args);
    ~Daedalus();
    
    void attach_shell(Shell &shell);
    void detach_shell();
    
    void attach_swapchain();
    void detach_swapchain();
    
    void on_key(Key key);
    void on_tick();
    
    void on_frame();
    
private:
    
    class Worker {
        
    public:
        
        Worker(Daedalus &daedalus, int index, int object_begin, int object_end);

        void start();
        void stop();
        void update_scene();
        void draw_objects(VkFramebuffer fb);
        void wait_idle();

        Daedalus &daedalus_;

        const int index_;
        const int object_begin_;
        const int object_end_;

        const float tick_interval_;

        VkFramebuffer frame_buffer_;

    private:
        
        enum State {
            INIT,
            IDLE,
            STEP,
            DRAW,
        };

        void update_loop();

        static void thread_loop(Worker *worker) { worker->update_loop(); }

        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable state_condition_variable_;
        State state_;
        
    };
    
    struct Camera {
        glm::vec3 eye_position;
        glm::mat4 view_projection;
        
        Camera(float eye) : eye_position(eye) {}
    };
    
    struct FrameData {
        VkFence fence;
        
        VkCommandBuffer primary_command;
        std::vector<VkCommandBuffer> worker_commands;
        
        VkBuffer buffer;
        uint8_t *base;
        VkDescriptorSet descriptor_set;
    };
    
    void init_workers();
    
    bool multithread_;
    bool use_push_constants_;
    
    // called mostly by on_key
    void update_camera();
    
    bool scene_paused_;
    Scene scene_;
    Camera camera_;
    
    std::vector<std::unique_ptr<Worker>> workers_;
    
    // called by attach_shell
    void create_render_pass();
    void create_shader_modules();
    void create_descriptor_set_layout();
    void create_pipeline_layout();
    void create_pipeline();
    
    void create_frame_data(int count);
    void destroy_frame_data();
    void create_fences();
    void create_command_buffers();
    void create_buffers();
    void create_buffer_memory();
    void create_descriptor_sets();
    
    VkPhysicalDevice physical_device_;
    VkDevice device_;
    VkQueue queue_;
    uint32_t queue_family_;
    VkFormat format_;
    VkDeviceSize aligned_object_data_size_;
    
    VkPhysicalDeviceProperties physical_device_properties_;
    std::vector<VkMemoryPropertyFlags> memory_flags_;
    
    const Meshes *meshes_;
    
    VkRenderPass render_pass_;
    VkShaderModule vertex_shader_;
    VkShaderModule fragment_shader_;
    VkDescriptorSetLayout descriptor_set_layout_;
    VkPipelineLayout pipeline_layout_;
    VkPipeline pipeline_;

    VkCommandPool primary_command_pool_;
    std::vector<VkCommandPool> worker_command_pools_;
    VkDescriptorPool descriptor_pool_;
    VkDeviceMemory frame_data_memory_;
    std::vector<FrameData> frame_data_;
    int frame_data_index_;

    VkClearValue render_pass_clear_value_;
    VkRenderPassBeginInfo render_pass_begin_info_;

    VkCommandBufferBeginInfo primary_command_begin_info_;
    VkPipelineStageFlags primary_command_submit_wait_stages_;
    VkSubmitInfo primary_command_submit_info_;

    // called by attach_swapchain
    void prepare_viewport(const VkExtent2D &extent);
    void prepare_framebuffers(VkSwapchainKHR swapchain);

    VkExtent2D extent_;
    VkViewport viewport_;
    VkRect2D scissor_;

    std::vector<VkImage> images_;
    std::vector<VkImageView> image_views_;
    std::vector<VkFramebuffer> framebuffers_;

    // called by workers
    void update_scene(const Worker &worker);
    void draw_object(const Scene::Object &object, FrameData &data, VkCommandBuffer command) const;
    void draw_objects(Worker &worker);
    
};
