#pragma once

#include <glm/glm.hpp>

#if defined(_WIN32)
    #include <Windows.h>
#elif defined(__APPLE__)
    #include <MoltenVK/mvk_vulkan.h>
#endif

#include "Platform.h"
#include "Types.h"
#include "Mesh.h"

#include "Explorer.h"
#include "Console.h"

#include <vector>
#include <string>
#include <deque>

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 renderMatrix;
};

struct DeletionQueue {
    std::deque<std::function<void()>> deletors;
    
    void push_function(std::function<void()>&& function) { deletors.push_back(function); }

    void flush() {
        // Reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
            (*it)(); // Call functions
        }
        deletors.clear();
    }
};

class Engine {
public:
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    
    Engine(const std::vector<std::string> &args, void *windowHandle);
    ~Engine();
    
    Explorer *explorer;
    Console console;
    void check(std::string section, std::string action, bool result, bool silentSuccess = false);
    void check(std::string section, std::string action, VkResult result, bool silentSuccess = false);
    
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
    
    VkShaderModule vertexShader;
    VkShaderModule fragmentShader;
    
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    Mesh triangleMesh;
    Mesh monkeyMesh;
    
    VkImageView depthImageView;
    AllocatedImage depthImage;

    VkFormat depthFormat;
    
    VmaAllocator allocator;
    DeletionQueue mainDeletionQueue;
    
    bool isInitialized{false};
    int frameNumber{0};
    
    struct Settings {
        const char* engineName;
        const char* applicationName;
        
        VkExtent2D windowExtent;
        
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
    void* windowHandle;
    
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
