#pragma once

#include <glm/glm.hpp>

#if defined(_WIN32)
    #include <Windows.h>
#elif defined(__APPLE__)
    #include <MoltenVK/mvk_vulkan.h>
#endif

#include "Engine.h"
#include "Platform.h"
#include "Bootstrap.h"
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

class Daedalus : public Engine {
public:
    Daedalus(const Daedalus &engine) = delete;
    Daedalus &operator = (const Daedalus &engine) = delete;
    ~Daedalus();
    
    Daedalus(const char* applicationName, const std::vector<std::string>& args, void* windowHandle);
    
    Explorer *explorer;
    Console console;
    void check(std::string section, std::string action, bool result, bool silentSuccess = false);
    void check(std::string section, std::string action, VkResult result, bool silentSuccess = false);
    
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;
    
    vkb::Swapchain swapchain_;
    uint32_t swapchainImageIndex;
    
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

    void onKey(Key key);
    
    void initialize();
    void update();
    
private:
    void* windowHandle;
    
    void initializeVulkan();
    void initializeSwapchain();
    void initializeCommands();
    void initializeDefaultRenderPass();
    void initializeFramebuffers();
    void initializeSyncStructures();
    void initializePipelines();
    
    void acquireBackBuffer();
    void presentBackBuffer();
    
    void onTick();
    void onFrame();
    
    void updateSwapchain();
    
    void terminate();
    void terminateSwapchain();
    
    bool loadShaderModule(const char* filePath, VkShaderModule* shaderModule);
    void loadMeshes();
    
    void uploadMesh(Mesh &mesh);
};
