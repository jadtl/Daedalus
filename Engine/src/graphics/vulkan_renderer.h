#pragma once

#include <core/defines.h>
#include <core/types.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <optional>
#include <vector>
#include <string>
#include <array>

namespace ddls {
/**
 * @brief The Vulkan renderer
 * 
 */
class DDLS_API VulkanRenderer
{
public:
    VulkanRenderer(GLFWwindow *window, const char* appName, const char* engineName);
    ~VulkanRenderer();
    void render(ImDrawData *drawData = nullptr);
    VkInstance instance() const { return _instance; }
    VkPhysicalDevice physicalDevice() const { return _physicalDevice; }
    VkDevice device() const { return _device; }
    VkQueue queue() const { return _graphicsQueue; }
    VkRenderPass renderPass() const { return _renderPass; }
    VkRenderPass renderPassImGui() const { return _renderPassImGui; }
    VkDescriptorPool descriptorPoolImGui() const { return _descriptorPoolImGui; }
    VkCommandPool commandPoolImGui() const { return _commandPoolImGui; }
    VkCommandBuffer commandBufferImGui() const { return _commandBuffersImGui[_currentFrame]; }
    u32 imageCount() const { return (u32)_swapchainImageViews.size(); }
    void setFramebufferResized() { _framebufferResized = true; }
    bool wireframe;
    f32 red;
    f32 green;
    f32 blue;
    f32 rotate;
private:
    void recreateSwapchain();
    void createSwapchain();
    void createSwapchainImageViews();
    void createSwapchainFramebuffers();
    void destroySwapchain();

    GLFWwindow *_window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice;
    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;
        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    VkDevice _device;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkSurfaceKHR _surface;
    struct swapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats; std::vector<VkPresentModeKHR> presentModes;
    };
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;
    std::vector<VkImageView> _swapchainImageViews;
    VkPipelineLayout _pipelineLayout;
    VkRenderPass _renderPass;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
    VkPipeline _graphicsPipeline;
    std::vector<VkFramebuffer> _swapchainFramebuffers;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    const u32 _MaxFramesInFlight = 2;
    u32 _currentFrame;
    b8 _framebufferResized;
    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};

            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }
        
        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            // vec2
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            // vec3
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };
    const std::vector<Vertex> _vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    VkBuffer _vertexBuffer;
    VkDeviceMemory _vertexBufferMemory;
    const std::vector<u16> _indices = {
        0, 1, 2, 2, 3, 0
    };
    VkBuffer _indexBuffer;
    VkDeviceMemory _indexBufferMemory;
    struct UniformBufferObject {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };
    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
    std::vector<void*> _uniformBuffersMapped;

    const std::vector<const char*> _validationLayers = 
        {"VK_LAYER_KHRONOS_validation"};
#ifdef DDLS_DEBUG
    const bool _enableValidationLayers = true;
    VkDebugUtilsMessengerEXT _debugMessenger;
#else
    const bool _enableValidationLayers = false;
#endif
    std::vector<const char*> _deviceExtensions =
        {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<const char*> getRequiredExtensions(
        VkInstanceCreateInfo *instanceCreateInfo,
        bool enableValidationLayers);
    void enumerateAvailableExtensions();
    bool checkValidationLayerSupport(
        std::vector<const char*> validationLayers);
    void fillDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo, 
        PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

    bool isDeviceSuitable(
        VkPhysicalDevice physicalDevice);
    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice physicalDevice);
    bool checkDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice);

    swapchainSupportDetails querySwapChainSupport(
        VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities);

    std::vector<char> readFile(const std::string& fileName);
    VkShaderModule createShaderModule(std::vector<char> code);

    u32 findMemoryType(
        u32 typeFilter,
        VkMemoryPropertyFlags properties);
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);
    void copyBuffer(
        VkBuffer dstBuffer,
        VkBuffer srcBuffer,
        VkDeviceSize size
    );

    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);
    void updateUniformBuffer(u32 currentFrame);

    void createDescriptorPoolImGui();
    void createCommandsImGui();
    void createRenderPassImGui();
    void createFramebuffersImGui();

    std::vector<VkCommandBuffer> submitBuffer;

    VkRenderPass _renderPassImGui;
    VkDescriptorPool _descriptorPoolImGui;
    std::vector<VkFramebuffer> _framebuffersImGui;
    VkCommandPool _commandPoolImGui;
    std::vector<VkCommandBuffer> _commandBuffersImGui;

    void recordImGui(
        VkCommandBuffer commandBuffer, 
        u32 imageIndex,
        ImDrawData *drawData);

    void createCommandPool(
        VkCommandPool *commandPool, 
        VkCommandPoolCreateFlags flags);
    void createCommandBuffers(
        VkCommandBuffer *commandBuffer, 
        u32 commandBufferCount, 
        VkCommandPool& commandPool);

    template <typename F>
    void record(VkRenderPass renderPass, VkCommandBuffer commandBuffer, F&& drawCalls);
    void submit();
    void present();

    VkPipeline _graphicsPipelineWireframe;
};
}
