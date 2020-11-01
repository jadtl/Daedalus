#pragma once

#include <vulkan/vulkan.hpp>

#include "QueueFamily.hpp"

#include <set>
#include <vector>

namespace ddls
{
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void pickPhysicalDevice(VkInstance instance,
                        VkPhysicalDevice& device,
                        const VkSurfaceKHR& surface);

bool isDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);

bool checkDeviceExtensionSupport(VkPhysicalDevice device);
}
