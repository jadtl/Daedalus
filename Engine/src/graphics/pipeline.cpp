#include "graphics/pipeline.h"

#include <core/log.h>

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
    VkPolygonMode polygonMode) : _device(device), _swapchain(swapchain), _renderPass(renderPass), _pipelineLayout(pipelineLayout)
{
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShader;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;
    _shaderStages.push_back(vertShaderStageInfo);

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShader;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;
    _shaderStages.push_back(fragShaderStageInfo);

    auto bindingDescription = vk::Vertex::getBindingDescription();
    auto attributeDescriptions = vk::Vertex::getAttributeDescriptions();
    _vertexInputInfo = {};
    _vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    _vertexInputInfo.vertexBindingDescriptionCount = 1;
    _vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    _vertexInputInfo.vertexAttributeDescriptionCount = (u32)attributeDescriptions.size();
    _vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    _inputAssembly = {};
    _inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    _inputAssembly.topology = topology;
    _inputAssembly.primitiveRestartEnable = VK_FALSE;

    _viewport = {};
    _viewport.x = 0.0f;
    _viewport.y = 0.0f;
    _viewport.width = (f32)_swapchain->extent().width;
    _viewport.height = (f32)_swapchain->extent().height;
    _viewport.minDepth = 0.0f;
    _viewport.maxDepth = 1.0f;

    _scissor = {};
    _scissor.offset = {0, 0};
    _scissor.extent = _swapchain->extent();

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &_viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &_scissor;

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

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &_colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    _pipelineInfo = {};
    _pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    _pipelineInfo.stageCount = (u32)_shaderStages.size();
    _pipelineInfo.pStages = _shaderStages.data();
    _pipelineInfo.pVertexInputState = &_vertexInputInfo;
    _pipelineInfo.pInputAssemblyState = &_inputAssembly;
    _pipelineInfo.pViewportState = &viewportState;
    _pipelineInfo.pRasterizationState = &_rasterizer;
    _pipelineInfo.pMultisampleState = &_multisampling;
    _pipelineInfo.pDepthStencilState = nullptr;
    _pipelineInfo.pColorBlendState = &colorBlending;
    _pipelineInfo.pDynamicState = &dynamicState;
    _pipelineInfo.layout = pipelineLayout;
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

void ddls::Pipeline::recreate()
{
    vkDestroyPipeline(_device, _pipeline, nullptr);
    _viewport.width = _swapchain->extent().width;
    _viewport.height = _swapchain->extent().height;
    _scissor.extent = _swapchain->extent();

    Assert(vkCreateGraphicsPipelines(_device, nullptr, 1, &_pipelineInfo, nullptr, &_pipeline),
        "Failed to recreate pipeline!");
}

}