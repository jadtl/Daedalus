#pragma once

#include <vulkan/vulkan.hpp>
#include "CommandBuffers.hpp"
#include "ImageViews.hpp"

#include <stdexcept>

namespace ddls
{
void createImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

void createTextureImage(VkPhysicalDevice physicalDevice, VkDevice device, VkImage& textureImage, VkDeviceMemory& textureImageMemory, VkCommandPool commandPool, VkQueue graphicsQueue);

void createTextureImageView(VkDevice device, VkImage& textureImage, VkImageView& textureImageView);

void createTextureSampler(VkDevice device, VkSampler& textureSampler);
}
