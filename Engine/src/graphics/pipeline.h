#pragma once

#include "core/defines.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <array>

namespace ddls
{
class Swapchain;
class DDLS_API Pipeline
{
public:
    Pipeline(
        VkDevice device,
        ddls::Swapchain *swapchain,
        VkRenderPass renderPass,
        VkPipelineLayout pipelineLayout,
        VkPrimitiveTopology topology,
        VkShaderModule vertexShader,
        VkShaderModule fragmentShader,
        VkPolygonMode polygonMode);
    ~Pipeline();
    VkPipeline handle() const { return _pipeline; }
    void recreate(const Swapchain *swapchain);
private:
    VkDevice _device;
    VkRenderPass _renderPass;
    VkPipeline _pipeline;
    VkGraphicsPipelineCreateInfo _pipelineInfo;
    VkPipelineShaderStageCreateInfo _vertexShaderStageInfo{};
    VkPipelineShaderStageCreateInfo _fragmentShaderStageInfo{};
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    std::vector<VkDynamicState> _dynamicStates;
	VkViewport _viewport;
	VkRect2D _scissor;
    VkPipelineDynamicStateCreateInfo _dynamicState;
    VkPipelineViewportStateCreateInfo _viewportState;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
    VkPipelineColorBlendStateCreateInfo _colorBlending;
    VkVertexInputBindingDescription _bindingDescription;
    std::array<VkVertexInputAttributeDescription, 2> _attributeDescriptions;
};
}