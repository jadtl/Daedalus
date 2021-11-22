#pragma once

#include <vector>
#include <string>
#include <deque>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "shell.h"
#include "renderer/types.h"
#include "renderer/mesh.h"

struct Material
{
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
};

struct RenderObject
{
    Mesh *mesh;

    Material *material;

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

    void push_function(std::function<void()> &&function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); //call functors
        }
        deletors.clear();
    }
};

class Engine
{
public:
    Engine(const Engine &engine) = delete;
    Engine &operator=(const Engine &engine) = delete;

    Engine(const std::vector<std::string> &args);
    ~Engine();

    GLFWwindow *window;

    Shell shell;

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

    Material *create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name);

    Material *get_material(const std::string &name);

    Mesh *get_mesh(const std::string &name);

    void draw_objects(VkCommandBuffer cmd, RenderObject *first, int count);

    bool isInitialized{false};

    int frameNumber{0};

    struct Settings
    {
        std::string engineName = "Daedalus";
        std::string applicationName = "Daedalus";

        VkExtent2D windowExtent;
        int selectedShader;

        bool validate{false};
        bool verbose{false};
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
    void onKey(Key key);

    void initialize();

    void update();
    void render();

private:
    void initializeWindow();

    void initializeVulkan();
    void initializeSwapchain();
    void initializeCommands();
    void initializeDefaultRenderPass();
    void initializeFramebuffers();
    void initializeSyncStructures();
    void initializePipelines();

    void terminate();
    void terminateSwapchain();

    void updateSwapchain();

    bool loadShaderModule(const char *filePath, VkShaderModule *shaderModule);
    void loadMeshes();
    void init_scene();

    void uploadMesh(Mesh &mesh);
};
