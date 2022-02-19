#pragma once

#include <defines.h>

#include <core/file.h>

#include <renderer/types.h>
#include <renderer/mesh.h>

#include <world/camera.h>

#include <utils/unique.h>

#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace ddls {
    struct Material
    {
        VkPipeline pipeline;
        VkPipelineLayout pipeline_layout;
    };

    struct RenderObject
    {
        Mesh* mesh;
        Material* material;
        glm::mat4 transform_matrix;
    };

    struct MeshPushConstants
    {
        glm::vec4 data;
        glm::mat4 render_matrix;
    };

    struct DeletionQueue
    {
        std::deque<std::function<void()>> deletors;

        void push_function(std::function<void()>&& function)
        {
            deletors.push_back(function);
        }

        void flush()
        {
            // Reverse iterate the deletion queue to execute all the functions
            for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
            {
                (*it)(); // Call functors
            }
            deletors.clear();
        }
    };

    /**
     * @brief An engine class used by Daedalus, {@link Engine#initialize} must be called to start the engine.
     */
    class DDLS_API Engine : Unique {
    public:
        Engine(const std::vector<std::string>& args);
        ~Engine();

        GLFWwindow* window;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkSurfaceKHR surface;

        VkSwapchainKHR swapchain;
        VkFormat swapchain_image_format;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_views;

        VkQueue graphics_queue;
        uint32_t graphics_queue_family;

        VkCommandPool command_pool;
        VkCommandBuffer main_command_buffer;

        VkRenderPass render_pass;

        std::vector<VkFramebuffer> framebuffers;

        VkSemaphore present_semaphore, render_semaphore;
        VkFence render_fence;

        VkPipelineLayout pipeline_layout;
        VkPipeline triangle_pipeline;
        VkPipeline colored_triangle_pipeline;

        VkShaderModule colored_triangle_frag_shader;
        VkShaderModule colored_triangle_vertex_shader;
        VkShaderModule triangle_frag_shader;
        VkShaderModule triangle_vertex_shader;
        VkShaderModule mesh_vertex_shader;

        VmaAllocator allocator;

        VkPipeline mesh_pipeline;
        VkPipelineLayout mesh_pipeline_layout;
        Mesh triangle_mesh;
        Mesh monkey_mesh;

        VkImageView depth_image_view;
        AllocatedImage depth_image;

        VkFormat depth_format;

        DeletionQueue main_deletion_queue;

        std::vector<RenderObject> renderables;

        std::unordered_map<std::string, Material> materials;
        std::unordered_map<std::string, Mesh> meshes;

        Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

        Material* get_material(const std::string& name);

        Mesh* get_mesh(const std::string& name);

        void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);

        bool is_initialized{ false };

        int frame_number{ 0 };

        struct Settings
        {
            std::string engine_name = "Daedalus";
            std::string application_name = "Daedalus";

            VkExtent2D window_extent;
            int selected_shader;

            bool validate{ false };
            bool verbose{ false };
        };
        Settings settings;

        enum Key
        {
            KEY_A,
            KEY_S,
            KEY_D,
            KEY_Q,
            KEY_W,
            KEY_E,
            KEY_SPACE,
        };
        void on_key(Key key);

        void print_welcome();

        void initialize();

        void update();
        void render();

    private:
        void initialize_window();

        void initialize_vulkan();
        void initialize_swapchain();
        void initialize_commands();
        void initialize_default_renderpass();
        void initialize_framebuffers();
        void initialize_sync_structures();
        void initialize_pipelines();

        void terminate();
        void terminate_swapchain();

        void update_swapchain();

        bool load_shader_module(const char* file_path, VkShaderModule* shader_module);
        void load_meshes();
        void init_scene();

        void upload_mesh(Mesh& mesh);
    };
}