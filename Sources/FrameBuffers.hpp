#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
void createFrameBuffers(VkDevice device, std::vector<VkFramebuffer>& swapChainFramebuffers, VkRenderPass renderPass,
                        std::vector<VkImageView> swapChainImageViews, VkExtent2D swapChainExtent, VkImageView depthImageView);
}
