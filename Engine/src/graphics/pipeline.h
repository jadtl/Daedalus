#pragma once

#include "core/defines.h"
#include "graphics/swapchain.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace ddls
{
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
    void recreate();
private:
    VkDevice _device;
    ddls::Swapchain *_swapchain;
    VkRenderPass _renderPass;
    VkPipeline _pipeline;
    VkGraphicsPipelineCreateInfo _pipelineInfo;
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
};
}