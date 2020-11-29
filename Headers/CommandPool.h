#pragma once

#include <vulkan/vulkan.hpp>

#include "QueueFamily.h"

namespace ddls
{
void createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool& commandPool);
}
