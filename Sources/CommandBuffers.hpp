#pragma once

#include <vulkan/vulkan.hpp>

#include "VertexBuffers.hpp"
#include "IndexBuffers.hpp"
#include "UniformBuffers.hpp"

namespace ddls
{
void createCommandBuffers(VkDevice device, std::vector<VkCommandBuffer>& commandBuffers, VkCommandPool commandPool,
                          VkPipeline graphicsPipeline, std::vector<VkFramebuffer> swapChainFramebuffers,
                          VkRenderPass renderPass, VkExtent2D swapChainExtent, VkBuffer vertexBuffer, VkBuffer indexBuffer,
                          VkPipelineLayout& pipelineLayout, std::vector<VkDescriptorSet>& descriptorSets);

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue graphicsQueue);

void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
}
