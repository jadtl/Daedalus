#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "QueueFamily.hpp"

namespace ddls
{
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

void createSwapChain(VkDevice& device, VkPhysicalDevice& physicalDevice,
                     VkSurfaceKHR &surface, VkSwapchainKHR& swapChain,
                     std::vector<VkImage> &swapChainImages, VkFormat &swapChainImageFormat,
                     VkExtent2D &swapChainExtent, GLFWwindow *window,
                     const uint32_t screenWidth, const uint32_t screenHeight);

VkSurfaceFormatKHR chooseSwapSurfaceFormat
(const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR chooseSwapPresentMode
(const std::vector<VkPresentModeKHR>& availablePresentModes);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            GLFWwindow *window,
                            const uint32_t screenWidth, const uint32_t screenHeight);

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR& surface);
}
