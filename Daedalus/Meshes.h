#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

class Meshes {
    
public:
    
    Meshes(VkDevice device, const std::vector<VkMemoryPropertyFlags> &memory_flags);
    ~Meshes();
    
    const VkPipelineVertexInputStateCreateInfo &vertex_input_state() const { return vertex_input_state_; }
    const VkPipelineInputAssemblyStateCreateInfo &input_assembly_state() const { return input_assembly_state_; }
    
    enum Type {
        MESH_TRIANGLE
    };
    
    void command_bind_buffers(VkCommandBuffer command) const;
    void command_draw(VkCommandBuffer command, Type type) const;
    
private:
    
    void allocate_resources(VkDeviceSize vertex_buffer_size, VkDeviceSize index_buffer_size, const std::vector<VkMemoryPropertyFlags> &memory_flags);

    VkDevice device_;

    VkVertexInputBindingDescription vertex_input_binding_;
    std::vector<VkVertexInputAttributeDescription> vertex_input_attributes_;
    VkPipelineVertexInputStateCreateInfo vertex_input_state_;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_;
    VkIndexType index_type_;

    std::vector<VkDrawIndexedIndirectCommand> draw_commands_;

    VkBuffer vertex_buffer_;
    VkBuffer index_buffer_;
    VkDeviceMemory memory_;
    VkDeviceSize index_buffer_memory_offset_;
    
};
