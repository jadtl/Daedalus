#pragma once

#include "CommandBuffers.hpp"

namespace ddls
{
void createIndexBuffers(VkPhysicalDevice physicalDevice, VkDevice device, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory, VkCommandPool& commandPool, VkQueue graphicsQueue, std::vector<uint32_t> indices);
}
