#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
VkImageView createImageView(VkDevice device, VkImage image, VkFormat format);

void createImageViews(VkDevice device, std::vector<VkImageView>& swapChainImageViews,
                      std::vector<VkImage> swapChainImages, VkFormat swapChainImageFormat);
}
