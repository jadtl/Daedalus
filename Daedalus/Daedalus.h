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
    
    void attachShell(Shell &shell);
    void detachShell();
    
    void attachSwapchain();
    void detachSwapchain();
    
    void onKey(Key key);
    void onTick();
    
    void onFrame();
    
private:
    
    class Worker {
        
    public:
        
        Worker(Daedalus &daedalus, int index, int objectBegin, int objectEnd);

        void start();
        void stop();
        void updateScene();
        void drawObjects(VkFramebuffer frameBuffer);
        void waitIdle();

        Daedalus &daedalus_;

        const int index_;
        const int objectBegin_;
        const int objectEnd_;

        const float tickInterval_;

        VkFramebuffer frameBuffer_;

    private:
        
        enum State {
            INIT,
            IDLE,
            STEP,
            DRAW,
        };

        void updateLoop();

        static void threadLoop(Worker *worker) { worker -> updateLoop(); }

        std::thread thread_;
        std::mutex mutex_;
        std::condition_variable stateConditionVariable_;
        State state_;
        
    };
    
    struct Camera {
        glm::vec3 eyePosition;
        glm::mat4 viewProjection;
        
        Camera(float eye) : eyePosition(eye) {}
    };
    
    struct FrameData {
        VkFence fence;
        
        VkCommandBuffer primaryCommand;
        std::vector<VkCommandBuffer> workerCommands;
        
        VkBuffer buffer;
        uint8_t *base;
        VkDescriptorSet descriptorSet;
    };
    
    void initWorkers();
    
    bool multithread_;
    bool usePushConstants_;
    
    // called mostly by on_key
    void updateCamera();
    
    bool scenePaused_;
    Scene scene_;
    Camera camera_;
    
    std::vector<std::unique_ptr<Worker>> workers_;
    
    // called by attach_shell
    void createRenderPass();
    void createShaderModules();
    void createDescriptorSetLayout();
    void createPipelineLayout();
    void createPipeline();
    
    void createFrameData(int count);
    void destroyFrameData();
    void createFences();
    void createCommandBuffers();
    void createBuffers();
    void createBufferMemory();
    void createDescriptorSets();
    
    VkPhysicalDevice physicalDevice_;
    VkDevice device_;
    VkQueue queue_;
    uint32_t queueFamily_;
    VkFormat format_;
    VkDeviceSize alignedObjectDataSize_;
    
    VkPhysicalDeviceProperties physicalDeviceProperties_;
    std::vector<VkMemoryPropertyFlags> memoryFlags_;
    
    const Meshes *meshes_;
    
    VkRenderPass renderPass_;
    VkShaderModule vertexShader_;
    VkShaderModule fragmentShader_;
    VkDescriptorSetLayout descriptorSetLayout_;
    VkPipelineLayout pipelineLayout_;
    VkPipeline pipeline_;

    VkCommandPool primaryCommandPool_;
    std::vector<VkCommandPool> workerCommandPools_;
    VkDescriptorPool descriptorPool_;
    VkDeviceMemory frameDataMemory_;
    std::vector<FrameData> frameData_;
    int frameDataIndex_;

    VkClearValue renderPassClearValue_;
    VkRenderPassBeginInfo renderPassBeginInfo_;

    VkCommandBufferBeginInfo primaryCommandBeginInfo_;
    VkPipelineStageFlags primaryCommandSubmitWaitStages_;
    VkSubmitInfo primaryCommandSubmitInfo_;

    // called by attach_swapchain
    void prepareViewport(const VkExtent2D &extent);
    void prepareFramebuffers(VkSwapchainKHR swapchain);

    VkExtent2D extent_;
    VkViewport viewport_;
    VkRect2D scissor_;

    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
    std::vector<VkFramebuffer> framebuffers_;

    // called by workers
    void updateScene(const Worker &worker);
    void drawObject(const Scene::Object &object, FrameData &data, VkCommandBuffer command) const;
    void drawObjects(Worker &worker);
    
};
