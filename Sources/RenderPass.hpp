#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
void createRenderPass(VkDevice device, VkPhysicalDevice physicalDevice, VkRenderPass& renderPass, VkFormat swapChainImageFormat);
}
