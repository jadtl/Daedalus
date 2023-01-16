#include "graphics/swapchain.h"

#include "graphics/pipeline.h"
#include "core/log.h"

namespace ddls
{
Swapchain::Swapchain(
    GLFWwindow *window,
    VkPhysicalDevice physicalDevice, 
    VkDevice device,
    VkSurfaceKHR surface) : _window(window), _physicalDevice(physicalDevice), _device(device), _surface(surface)
{
    createSwapchain();
    createSwapchainImageViews();
}

Swapchain::~Swapchain()
{
    destroySwapchain();
}

void Swapchain::recreate()
{
    i32 width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_device);

    destroySwapchain();

    createSwapchain();
    createSwapchainImageViews();

    for (auto pair: _framebuffersCallbacks)
    {
        for (u32 i = 0; i < (u32)_imageViews.size(); i++)
            vkDestroyFramebuffer(_device, pair.first[i], nullptr);
        vk::createFramebuffers(
            _device,
            pair.first,
            (u32)_imageViews.size(),
            pair.second,
            _extent,
            _imageViews.data());
    }

    for (auto pipeline : _pipelineCallbacks)
    {
        pipeline->recreate(this);
    }
}

void Swapchain::createSwapchain()
{
    vk::SwapchainSupportDetails swapChainSupport = vk::querySwapchainSupport(_physicalDevice, _surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Adding one reduces internal waiting
    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = _surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    // Always 1 unless developing a stereoscoping 3D application
    swapchainCreateInfo.imageArrayLayers = 1;
    // Rendering directly to the images, i.e. not doing any post-processing
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    vk::QueueFamilyIndices indices = vk::findQueueFamilies(_physicalDevice, _surface);
    u32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    // No pre-transform
    swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // Ignore the alpha channel
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    Assert(vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain) == VK_SUCCESS,
        "Failed to create swapchain!");

    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
    _images.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _images.data());

    _format = surfaceFormat.format;
    _extent = extent;
}

void Swapchain::createSwapchainImageViews()
{
    _imageViews.resize(_images.size());

    for (u32 i = 0; i < _images.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = _images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = _format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        Assert(vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_imageViews[i]) == VK_SUCCESS,
            "Failed to create swapchain image view!");
    }
}

void Swapchain::destroySwapchain()
{
    for (size_t i = 0; i < _imageViews.size(); i++)
        vkDestroyImageView(_device, _imageViews[i], nullptr);

    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}
}