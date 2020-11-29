#pragma once

#include <vulkan/vulkan.hpp>

#include "VertexBuffers.h"

#include <fstream>

namespace ddls
{
void createGraphicsPipeline(VkDevice device, VkPipelineLayout& pipelineLayout, VkPipeline& graphicsPipeline,
                            VkRenderPass renderPass, const VkExtent2D& swapChainExtent, VkDescriptorSetLayout& descriptorSetLayout);

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file!");
    }
    
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    
    file.close();

    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
}
