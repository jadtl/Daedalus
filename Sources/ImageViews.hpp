#pragma once

#include <vulkan/vulkan.hpp>

namespace ddls
{
void createImageViews(VkDevice device, std::vector<VkImageView>& swapChainImageViews,
                      std::vector<VkImage> swapChainImages, VkFormat swapChainImageFormat);
}
