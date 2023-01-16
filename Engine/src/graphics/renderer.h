#pragma once

#include <core/defines.h>
#include <core/types.h>
#include <graphics/swapchain.h>

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
class DDLS_API Renderer
{
public:
    Renderer(GLFWwindow *window, const char* appName, const char* engineName);
    ~Renderer();
    void draw(ImDrawData *drawData = nullptr);
    VkInstance instance() const { return _instance; }
    VkPhysicalDevice physicalDevice() const { return _physicalDevice; }
    VkDevice device() const { return _device; }
    Swapchain *swapchain() const { return _swapchain.get(); }
    VkQueue queue() const { return _graphicsQueue; }
    u32 queueFamilyIndex() const { return _indices.graphicsFamily.value(); }
    void setFramebufferResized() { _framebufferResized = true; }
    u32 MaxFramesInFlight() const { return _MaxFramesInFlight; }
    bool wireframe;
    f32 red;
    f32 green;
    f32 blue;
    f32 rotate;
    u32 currentFrame() const { return _currentFrame; }
    u32 newFrame();
    void submit(VkCommandBuffer commandBuffer) { _submitBuffer.push_back(commandBuffer); }
    void render(u32 imageIndex);
private:
    GLFWwindow *_window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice;
    std::unique_ptr<Swapchain> _swapchain;
    VkDevice _device;
    vk::QueueFamilyIndices _indices;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkSurfaceKHR _surface;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;
    std::vector<VkDescriptorSet> _descriptorSets;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    const u32 _MaxFramesInFlight = 2;
    u32 _currentFrame;
    b8 _framebufferResized;

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
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface);
    bool checkDeviceExtensionSupport(
        VkPhysicalDevice physicalDevice);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);

    std::vector<VkCommandBuffer> _submitBuffer;
};
}