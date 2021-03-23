#pragma once

#include <vector>
#include <string>
#include <deque>

#include <MoltenVK/mvk_vulkan.h>
#include <glm/glm.hpp>

#include "Types.h"
#include "Mesh.h"

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 renderMatrix;
};

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;
    
    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); //call functors
        }
        deletors.clear();
    }
};

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
    
    
    DeletionQueue mainDeletionQueue;
    
    bool isInitialized{false};
    int frameNumber{0};
    
    struct Settings {
        std::string engineName = "Daedalus";
        std::string applicationName = "The Architect";
        
        VkExtent2D windowExtent;
        int selectedShader;
        
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
    
    void initialize();
    
    void update();
    void render();
    
private:
    void* caMetalLayer;
    
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
    
    bool loadShaderModule(const char* filePath, VkShaderModule* shaderModule);
    void loadMeshes();
    
    void uploadMesh(Mesh &mesh);
};
