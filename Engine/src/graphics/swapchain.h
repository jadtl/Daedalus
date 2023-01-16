#pragma once

#include "core/defines.h"
#include "core/types.h"
#include "utils/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <functional>


namespace ddls
{
class Pipeline;
/**
 * @brief A swapchain used by the renderer to present rendered images
 * 
 */
class DDLS_API Swapchain
{
public:
    Swapchain(
        GLFWwindow *window,
        VkPhysicalDevice physicalDevice, 
        VkDevice device,
        VkSurfaceKHR surface);
    ~Swapchain();
    void recreate();

    VkSwapchainKHR handle() const { return _swapchain; }
    vk::SwapchainSupportDetails supportDetails() const { return _supportDetails; }
    u32 imageCount() const { return (u32)_imageViews.size(); }
    VkFormat format() const { return _format; }
    VkExtent2D extent() const { return _extent; }
    std::vector<VkImageView> imageViews() const { return _imageViews; }
    void addFramebuffersRecreateCallback(VkFramebuffer* framebuffer, VkRenderPass renderPass) 
        { _framebuffersCallbacks.push_back(std::make_pair(framebuffer, renderPass)); }
    void addPipelineRecreateCallback(Pipeline *pipeline) 
        { _pipelineCallbacks.push_back(pipeline); };

private:
    GLFWwindow *_window;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkSurfaceKHR _surface;
    VkSwapchainKHR _swapchain;
    std::vector<VkImage> _images;
    VkFormat _format;
    VkExtent2D _extent;
    std::vector<VkImageView> _imageViews;
    vk::SwapchainSupportDetails _supportDetails;
    std::vector<std::pair<VkFramebuffer*, VkRenderPass> > _framebuffersCallbacks;
    std::vector<Pipeline*> _pipelineCallbacks;

    void createSwapchain();
    void createSwapchainImageViews();
    void destroySwapchain();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities);
};
}