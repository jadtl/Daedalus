#pragma once

#include <vector>
#include <string>
#include <deque>
#include <unordered_map>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Shell.h"
#include "Types.h"
#include "Mesh.h"

struct Material
{
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct RenderObject
{
    Mesh *mesh;

    Material *material;

    glm::mat4 transformMatrix;
};

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 renderMatrix;
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

    Engine(const std::vector<std::string> &args, void *caMetalLayer);
    ~Engine();

    GLFWwindow *window;

    Shell shell;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;

    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;

    VkRenderPass renderPass;

    std::vector<VkFramebuffer> framebuffers;

    VkSemaphore presentSemaphore, renderSemaphore;
    VkFence renderFence;

    VkPipelineLayout pipelineLayout;
    VkPipeline trianglePipeline;
    VkPipeline coloredTrianglePipeline;

    VkShaderModule coloredTriangleFragShader;
    VkShaderModule coloredTriangleVertexShader;
    VkShaderModule triangleFragShader;
    VkShaderModule triangleVertexShader;
    VkShaderModule meshVertexShader;

    VmaAllocator allocator;

    VkPipeline meshPipeline;
    VkPipelineLayout meshPipelineLayout;
    Mesh triangleMesh;
    Mesh monkeyMesh;

    VkImageView depthImageView;
    AllocatedImage depthImage;

    VkFormat depthFormat;

    DeletionQueue mainDeletionQueue;

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
    void *caMetalLayer;

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
