#pragma once

#include <core/defines.h>
#include <core/types.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

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
    void render();
private:
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
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _swapchainImages;
    VkFormat _swapchainImageFormat;
    VkExtent2D _swapchainExtent;
    std::vector<VkImageView> _swapchainImageViews;
    VkPipelineLayout _pipelineLayout;
    VkRenderPass _renderPass;
    VkPipeline _graphicsPipeline;
    std::vector<VkFramebuffer> _swapchainFramebuffers;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;

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

    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);
};
}