#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
void createRenderPass(VkDevice device, VkRenderPass& renderPass, VkFormat swapChainImageFormat);
}
