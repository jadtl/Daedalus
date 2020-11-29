#pragma once

#include <vulkan/vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ImageViews.h"
#include "TextureImage.h"
#include "CommandBuffers.h"

namespace ddls
{
void createDepthResources(VkDevice device, VkPhysicalDevice physicalDevice, VkImage& depthImage, VkDeviceMemory& depthImageMemory, VkImageView& depthImageView, VkExtent2D swapChainExtent);

VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

bool hasStencilComponent(VkFormat format);
}
