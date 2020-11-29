#pragma once

#include "CommandBuffers.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace ddls
{
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

void createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout& descriptorSetLayout);

void createDescriptorPool(VkDevice device, VkDescriptorPool& descriptorPool, std::vector<VkImage> swapChainImages);

void createDescriptorSets(VkDevice device, std::vector<VkBuffer> uniformBuffers, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, std::vector<VkDescriptorSet>& descriptorSets, std::vector<VkImage> swapChainImages, VkImageView textureImageView, VkSampler textureSampler);

void createUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, std::vector<VkBuffer>& uniformBuffers, std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<VkImage>& swapChainImages);

void updateUniformBuffer(VkDevice device, VkExtent2D swapChainExtent, std::vector<VkDeviceMemory>& uniformBuffersMemory, uint32_t currentImage);
}
