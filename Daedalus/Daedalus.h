#pragma once

#include <glm/glm.hpp>

#include "Types.h"
#include "Engine.h"
#include "Shell.h"
#include "Mesh.h"

#include <vector>
#include <string>
#include <deque>
#include <functional>

class Shell;

class Daedalus : public Engine {
public:
    Daedalus(const char* applicationName, const std::vector<std::string>& args);
    ~Daedalus();

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
    
    void onTick();
    void onFrame();
    
    void onKey(Key key);
    
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamily;
    
    vkb::Swapchain vkbSwapchain;
    
    VkImageView depthImageView;
    AllocatedImage depthImage;
    VkFormat depthFormat;
    
    VkRenderPass renderPass;
    VkCommandPool commandPool;
    VkCommandBuffer mainCommandBuffer;
    std::vector<VkFramebuffer> framebuffers;
    
    VkShaderModule coloredTriangleFragShader;
    VkShaderModule coloredTriangleVertexShader;
    VkShaderModule triangleFragShader;
    VkShaderModule triangleVertexShader;
    VkShaderModule meshVertexShader;
    
    VkPipeline trianglePipeline;
    VkPipeline coloredTrianglePipeline;
    VkPipeline meshPipeline;
    VkPipelineLayout pipelineLayout;
    VkPipelineLayout meshPipelineLayout;
    Mesh triangleMesh;
    Mesh monkeyMesh;
    
    VmaAllocator allocator;
    DeletionQueue mainDeletionQueue;
    
private:
    void initializeCommands();
    void initializeDepthBuffers();
    void initializeDefaultRenderPass();
    void initializeFramebuffers();
    void initializeSyncStructures();
    void initializePipelines();
    //initialize image views in framebuffer

    bool loadShaderModule(const char* filePath, VkShaderModule* shaderModule);
    void loadMeshes();
    
    void uploadMesh(Mesh &mesh);
};
