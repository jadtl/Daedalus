#pragma once

#include <renderer/types.h>

#include <glm/vec3.hpp>

#include <vector>

struct VertexInputDescription
{
    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;

    VkPipelineVertexInputStateCreateFlags flags = 0;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static VertexInputDescription get_vertex_description();
};

struct Mesh
{
    std::vector<Vertex> vertices;
    AllocatedBuffer vertex_buffer;

    bool load_from_obj(const char* file_name);
};
