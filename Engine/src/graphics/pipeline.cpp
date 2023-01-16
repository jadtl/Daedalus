#include "graphics/pipeline.h"

#include "core/log.h"
#include "graphics/swapchain.h"

namespace ddls
{
Pipeline::Pipeline(
    VkDevice device,
    ddls::Swapchain *swapchain,
    VkRenderPass renderPass,
    VkPipelineLayout pipelineLayout,
    VkPrimitiveTopology topology,
    VkShaderModule vertexShader,
    VkShaderModule fragmentShader,
    VkPolygonMode polygonMode) : _device(device), _renderPass(renderPass), _pipelineLayout(pipelineLayout)
{
    _vertexShaderStageInfo = {};
    _vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    _vertexShaderStageInfo.module = vertexShader;
    _vertexShaderStageInfo.pName = "main";
    _vertexShaderStageInfo.pSpecializationInfo = nullptr;
    _shaderStages.push_back(_vertexShaderStageInfo);

    _fragmentShaderStageInfo = {};
    _fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    _fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    _fragmentShaderStageInfo.module = fragmentShader;
    _fragmentShaderStageInfo.pName = "main";
    _fragmentShaderStageInfo.pSpecializationInfo = nullptr;
    _shaderStages.push_back(_fragmentShaderStageInfo);

    _bindingDescription = vk::Vertex::getBindingDescription();
    _attributeDescriptions = vk::Vertex::getAttributeDescriptions();
    _vertexInputInfo = {};
    _vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    _vertexInputInfo.vertexBindingDescriptionCount = 1;
    _vertexInputInfo.pVertexBindingDescriptions = &_bindingDescription;
    _vertexInputInfo.vertexAttributeDescriptionCount = (u32)_attributeDescriptions.size();
    _vertexInputInfo.pVertexAttributeDescriptions = _attributeDescriptions.data();

    _inputAssembly = {};
    _inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    _inputAssembly.topology = topology;
    _inputAssembly.primitiveRestartEnable = VK_FALSE;

    _viewport = {};
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = (f32)swapchain->extent().width;
    _viewport.height = (f32)swapchain->extent().height;
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    _scissor = {};
    _scissor.offset = {0, 0};
    _scissor.extent = swapchain->extent();

    _dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    _dynamicState = {};
    _dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    _dynamicState.dynamicStateCount = static_cast<u32>(_dynamicStates.size());
    _dynamicState.pDynamicStates = _dynamicStates.data();

    _viewportState = {};
    _viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    _viewportState.viewportCount = 1;
    _viewportState.pViewports = &_viewport;
    _viewportState.scissorCount = 1;
    _viewportState.pScissors = &_scissor;

    _rasterizer = {};
    _rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _rasterizer.depthClampEnable = VK_FALSE;
    _rasterizer.rasterizerDiscardEnable = VK_FALSE;
    _rasterizer.polygonMode = polygonMode;
    _rasterizer.lineWidth = 1.0f;
    _rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // Counter-clockwise because of Y-flip in projection matrix
    _rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    _rasterizer.depthBiasEnable = VK_FALSE;
    _rasterizer.depthBiasConstantFactor = 0.0f;
    _rasterizer.depthBiasClamp = 0.0f;
    _rasterizer.depthBiasSlopeFactor = 0.0f;

    _multisampling = {};
    _multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    _multisampling.sampleShadingEnable = VK_FALSE;
    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f;
    _multisampling.pSampleMask = nullptr;
    _multisampling.alphaToCoverageEnable = VK_FALSE;
    _multisampling.alphaToOneEnable = VK_FALSE;

    _colorBlendAttachment = {};
    _colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    _colorBlendAttachment.blendEnable = VK_FALSE;
    _colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    _colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    _colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    _colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    _colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    _colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    _colorBlending = {};
    _colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    _colorBlending.logicOpEnable = VK_FALSE;
    _colorBlending.logicOp = VK_LOGIC_OP_COPY;
    _colorBlending.attachmentCount = 1;
    _colorBlending.pAttachments = &_colorBlendAttachment;
    _colorBlending.blendConstants[0] = 0.0f;
    _colorBlending.blendConstants[1] = 0.0f;
    _colorBlending.blendConstants[2] = 0.0f;
    _colorBlending.blendConstants[3] = 0.0f;

    _pipelineInfo = {};
    _pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _pipelineInfo.stageCount = (u32)_shaderStages.size();
    _pipelineInfo.pStages = _shaderStages.data();
    _pipelineInfo.pVertexInputState = &_vertexInputInfo;
    _pipelineInfo.pInputAssemblyState = &_inputAssembly;
    _pipelineInfo.pViewportState = &_viewportState;
    _pipelineInfo.pRasterizationState = &_rasterizer;
    _pipelineInfo.pMultisampleState = &_multisampling;
    _pipelineInfo.pDepthStencilState = nullptr;
    _pipelineInfo.pColorBlendState = &_colorBlending;
    _pipelineInfo.pDynamicState = &_dynamicState;
    _pipelineInfo.layout = _pipelineLayout;
    _pipelineInfo.renderPass = _renderPass;
    _pipelineInfo.subpass = 0;
    _pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    _pipelineInfo.basePipelineIndex = -1;

    Assert(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &_pipelineInfo, nullptr, &_pipeline) == VK_SUCCESS,
        "Failed to create graphics pipeline!");
}

Pipeline::~Pipeline()
{
    vkDestroyPipeline(_device, _pipeline, nullptr);
}

void ddls::Pipeline::recreate(const Swapchain *swapchain)
{
    vkDestroyPipeline(_device, _pipeline, nullptr);
    _viewport.width = (f32)swapchain->extent().width;
    _viewport.height = (f32)swapchain->extent().height;
    _scissor.extent = swapchain->extent();

    Assert(vkCreateGraphicsPipelines(_device, nullptr, 1, &_pipelineInfo, nullptr, &_pipeline) == VK_SUCCESS,
        "Failed to recreate pipeline!");
}

}