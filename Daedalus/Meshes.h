#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

class Meshes {
    
public:
    
    Meshes(VkDevice device, const std::vector<VkMemoryPropertyFlags> &memoryFlags);
    ~Meshes();
    
    const VkPipelineVertexInputStateCreateInfo &vertexInputState() const { return vertexInputState_; }
    const VkPipelineInputAssemblyStateCreateInfo &inputAssemblyState() const { return inputAssemblyState_; }
    
    enum Type {
        MESH_TRIANGLE
    };
    
    void command_bind_buffers(VkCommandBuffer command) const;
    void command_draw(VkCommandBuffer command, Type type) const;
    
private:
    
    void allocate_resources(VkDeviceSize vertexBufferSize, VkDeviceSize indexBufferSize, const std::vector<VkMemoryPropertyFlags> &memoryFlags);

    VkDevice device_;

    VkVertexInputBindingDescription vertexInputBinding_;
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributes_;
    VkPipelineVertexInputStateCreateInfo vertexInputState_;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState_;
    VkIndexType indexType_;

    std::vector<VkDrawIndexedIndirectCommand> drawCommands_;

    VkBuffer vertexBuffer_;
    VkBuffer indexBuffer_;
    VkDeviceMemory memory_;
    VkDeviceSize indexBufferMemoryOffset_;
    
};
