#include <renderer/pipeline.h>

#include <iostream>

VkPipeline PipelineBuilder::buildPipeline(VkDevice device, VkRenderPass render_pass) {
    //make viewport state from our stored viewport and scissor.
    //at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewport_state_info = {};
    viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_info.pNext = nullptr;

    viewport_state_info.viewportCount = 1;
    viewport_state_info.pViewports = &viewport;
    viewport_state_info.scissorCount = 1;
    viewport_state_info.pScissors = &scissor;

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo color_blending_info = {};
    color_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending_info.pNext = nullptr;

    color_blending_info.logicOpEnable = VK_FALSE;
    color_blending_info.logicOp = VK_LOGIC_OP_COPY;
    color_blending_info.attachmentCount = 1;
    color_blending_info.pAttachments = &color_blend_attachment;

    //build the actual pipeline
    //we now use all of the info structs we have been writing into into this one to create the pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.pNext = nullptr;

    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state_info;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending_info;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.layout = pipeline_layout;
    pipeline_info.renderPass = render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = nullptr;

    //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(
        device, nullptr, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
        std::cout << "Failed to create pipeline" << "\n";
        return VK_NULL_HANDLE; // Failed to create graphics pipeline
    }
    else { return pipeline; }
}
