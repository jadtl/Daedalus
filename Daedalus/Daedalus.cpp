#include "Daedalus.h"

#include <glm/gtx/transform.hpp>

#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"
#include "Pipeline.h"
#include "Engine.h"

#include <algorithm>
#include <iostream>
#include <fstream>

Daedalus::Daedalus(const char* applicationName, const std::vector<std::string>& args) 
    : Engine(applicationName, "Daedalus", args) {}

Daedalus::~Daedalus() {}

void Daedalus::attachShell(Shell& shell) {
    Engine::attachShell(shell);
    
    const Shell::Context &context = shell.context();
    this->instance = context.instance;
    this->physicalDevice = context.physicalDevice;
    this->device = context.device;
    this->graphicsQueue = context.graphicsQueue;
    this->graphicsQueueFamily = context.graphicsQueueFamily;
    
    this->allocator = context.allocator;
    
    loadMeshes();
}

void Daedalus::detachShell() {
    vkFreeCommandBuffers(device, commandPool, 1, &mainCommandBuffer);
    
    vkDestroyPipeline(device, trianglePipeline, nullptr);
    vkDestroyPipeline(device, coloredTrianglePipeline, nullptr);
    vkDestroyPipeline(device, meshPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyCommandPool(device, commandPool, nullptr);
    
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    vmaDestroyAllocator(allocator);
    
    Engine::detachShell();
}

void Daedalus::attachSwapchain(vkb::Swapchain vkbSwapchain) {
    this->vkbSwapchain = vkbSwapchain;
    
    initializeDepthBuffers();
    initializeDefaultRenderPass();
    initializePipelines();
    initializeFramebuffers();
    initializeCommands();
}

void Daedalus::detachSwapchain() {
    std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr); });
}

void Daedalus::acquireBackBuffer() {
    
}

void Daedalus::presentBackBuffer() {
    
}

void Daedalus::onTick() {
    //everything that happens in the world is updated here
}

void Daedalus::onFrame() {
    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
    vkResetCommandBuffer(mainCommandBuffer, 0) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Command buffer reset") : shell_->log(Shell::LOG_ERROR, "Command buffer failed to reset!");
    
    //naming it cmd for shorter writing
    VkCommandBuffer cmd = mainCommandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmd, &commandBufferBeginInfo) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Command buffer begins") : shell_->log(Shell::LOG_ERROR, "Command buffer failed to begin!");
    
    //make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    
    float fadeBlue = abs(sin(shell_->context().backBuffer.frameNumber / 25.f)) / 7.5f;
    clearValue.color = { { 0.f, 0.f, fadeBlue, 1.0f } };
    
    //clear depth at 1
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    //start the main renderpass.
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo renderPassInfo = init::renderPassBeginInfo(renderPass, vkbSwapchain.extent, framebuffers[shell_->context().swapchainImageIndex]);

    //connect clear values
    renderPassInfo.clearValueCount = 2;
    
    VkClearValue clearValues[] = { clearValue, depthClear };

    renderPassInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //drawing start
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPipeline);
    
    //make a model view matrix for rendering the object
    //camera position
    glm::vec3 cameraPosition = { 0.f, -2.f, -15.f };

    glm::mat4 view = glm::translate(glm::mat4(1.f), cameraPosition);
    //camera projection
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)(settings_.initialWindowExtent.width / settings_.initialWindowExtent.height), 0.1f, 100.0f);
    projection[1][1] *= -1;
    //model rotation
    glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(shell_->context().backBuffer.frameNumber * 1.f), glm::vec3(0, 1, 0));

    //calculate final mesh matrix
    glm::mat4 meshMatrix = projection * view * model;

    MeshPushConstants constants;
    constants.renderMatrix = meshMatrix;

    //upload the matrix to the GPU via pushconstants
    vkCmdPushConstants(cmd, meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
    
    //bind the mesh vertex buffer with offset 0
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &monkeyMesh.vertexBuffer.buffer, &offset);

    //we can now draw the mesh
    vkCmdDraw(cmd, monkeyMesh.vertices.size(), 1, 0, 0);
    
    vkCmdEndRenderPass(cmd);
    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    vkEndCommandBuffer(cmd) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Command buffer ends") : shell_->log(Shell::LOG_ERROR, "Command buffer failed to end!");;
    
    //prepare the submission to the queue.
    //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
    //we will signal the _renderSemaphore, to signal that rendering has finished

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &shell_->context().backBuffer.presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &shell_->context().backBuffer.renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    vkQueueSubmit(shell_->context().graphicsQueue, 1, &submit, shell_->context().backBuffer.renderFence) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Queue successfully submitted") : shell_->log(Shell::LOG_ERROR, "Queue failed to submit!");

    //increase the number of frames drawn
    frameNumber++;
}

void Daedalus::onKey(Key key) {
    switch (key) {
        case KEY_A:
            settings_.selectedShader = settings_.selectedShader == 0 ? 1 : 0;
            break;
        case KEY_S:
            
            break;
        case KEY_D:
            
            break;
        case KEY_Q:
            
            break;
        case KEY_W:
            
            break;
        case KEY_E:
            
            break;
        case Key::KEY_SPACE:
            settings_.selectedShader = settings_.selectedShader == 0 ? 1 : 0;
            break;
    }
}

void Daedalus::initializeCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = init::commandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Command pool successfully created") :
    shell_->log(Shell::LOG_ERROR, "Command pool failed to create!");
    
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = init::commandBufferAllocateInfo(commandPool, 1);

    vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &mainCommandBuffer) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Command buffers successfully allocated") :
    shell_->log(Shell::LOG_ERROR, "Command buffers failed to allocate!");
}

void Daedalus::initializeDepthBuffers() {
    //depth image size will match the window
    VkExtent3D depthImageExtent = {
        vkbSwapchain.extent.width,
        vkbSwapchain.extent.height,
        1
    };
    
    //hardcoding the depth format to 32 bit float
    depthFormat = VK_FORMAT_D32_SFLOAT;

    //the depth image will be an image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo depthImageInfo = init::imageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    //for the depth image, we want to allocate it from GPU local memory
    VmaAllocationCreateInfo depthImageAllocationInfo = {};
    depthImageAllocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depthImageAllocationInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(allocator, &depthImageInfo, &depthImageAllocationInfo, &depthImage.image, &depthImage.allocation, nullptr);

    //build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo depthViewInfo = init::imageViewCreateInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    vkCreateImageView(device, &depthViewInfo, nullptr, &depthImageView) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Image view successfully created") : shell_->log(Shell::LOG_ERROR, "Image view failed to create!");

    //add to deletion queues
    mainDeletionQueue.push_function([=]() {
        vkDestroyImageView(device, depthImageView, nullptr);
        shell_->log(Shell::LOG_INFO, "Image view destroyed");
        vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
        shell_->log(Shell::LOG_INFO, "Image destroyed");
    });
}

void Daedalus::initializeDefaultRenderPass() {
    // the renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment = {};
    //the attachment will have the format needed by the swapchain
    colorAttachment.format = vkbSwapchain.image_format;
    //1 sample, we won't be doing MSAA
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // we Clear when this attachment is loaded
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // we keep the attachment stored when the renderpass ends
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //we don't care about stencil
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    //we don't know or care about the starting layout of the attachment
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    //after the renderpass ends, the image has to be on a layout ready for display
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentReference = {};
    //attachment number will index into the pAttachments array in the parent renderpass itself
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentDescription depthAttachment = {};
    // Depth attachment
    depthAttachment.flags = 0;
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachementReference = {};
    depthAttachementReference.attachment = 1;
    depthAttachementReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //we are going to create 1 subpass, which is the minimum you can do
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    //hook the depth attachment into the subpass
    subpass.pDepthStencilAttachment = &depthAttachementReference;
    
    //array of 2 attachments, one for the color, and other for depth
    VkAttachmentDescription attachments[2] = { colorAttachment, depthAttachment };
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the color attachment to the info
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = &attachments[0];
    //connect the subpass to the info
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Render pass successfully created") :
    shell_->log(Shell::LOG_ERROR, "Render pass failed to create!");
}

void Daedalus::initializeFramebuffers() {
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = vkbSwapchain.extent.width;
    framebufferCreateInfo.height = vkbSwapchain.extent.height;
    framebufferCreateInfo.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchainImagecount = vkbSwapchain.get_images().value().size();
    framebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

    //create framebuffers for each of the swapchain image views
    for (int i = 0; i < swapchainImagecount; i++) {
        VkImageView attachments[2];
        attachments[0] = vkbSwapchain.get_image_views().value().at(i);
        attachments[1] = depthImageView;

        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.attachmentCount = 2;
        
        vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]) == VK_SUCCESS ?
        shell_->log(Shell::LOG_INFO, "Framebuffer successfully created") :
        shell_->log(Shell::LOG_ERROR, "Framebuffer failed to create!");
    }
}

void Daedalus::initializePipelines() {
    if (!loadShaderModule(shell_->explorer_.shader("ColoredTriangle.frag.spv").c_str(), &coloredTriangleFragShader)) {
        std::cout << "Error when building the triangle fragment shader module" << "\n";
    } else { std::cout << "Colored triangle fragment shader succesfully loaded" << "\n"; }

    if (!loadShaderModule(shell_->explorer_.shader("ColoredTriangle.vert.spv").c_str(), &coloredTriangleVertexShader)) {
        std::cout << "Error when building the triangle vertex shader module" << "\n";
    } else { std::cout << "Colored triangle vertex shader succesfully loaded" << "\n"; }
    
    if (!loadShaderModule(shell_->explorer_.shader("Triangle.frag.spv").c_str(), &triangleFragShader)) {
        std::cout << "Error when building the triangle fragment shader module" << "\n";
    } else { std::cout << "Triangle fragment shader succesfully loaded" << "\n"; }

    if (!loadShaderModule(shell_->explorer_.shader("Triangle.vert.spv").c_str(), &triangleVertexShader)) {
        std::cout << "Error when building the triangle vertex shader module" << "\n";
    } else { std::cout << "Triangle vertex shader succesfully loaded" << "\n"; }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = init::pipelineLayoutCreateInfo();
    
    vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Pipeline layout successfully created") :
    shell_->log(Shell::LOG_ERROR, "Pipeline layout failed to create!");
    
    //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
    PipelineBuilder pipelineBuilder;

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


    //vertex input controls how to read vertices from vertex buffers. We aren't using it yet
    pipelineBuilder.vertexInputInfo = init::vertexInputStateCreateInfo();
    
    //input assembly is the configuration for drawing triangle lists, strips, or individual points.
    //we are just going to draw triangle list
    pipelineBuilder.inputAssembly = init::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    //build viewport and scissor from the swapchain extents
    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float)vkbSwapchain.extent.width;
    pipelineBuilder.viewport.height = (float)vkbSwapchain.extent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;
    
    pipelineBuilder.scissor.offset = {0, 0};
    pipelineBuilder.scissor.extent = vkbSwapchain.extent;

    //configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = init::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    //we don't use multisampling, so just run the default one
    pipelineBuilder.multisampling = init::multisamplingStateCreateInfo();

    //a single blend attachment with no blending and writing to RGBA
    pipelineBuilder.colorBlendAttachment = init::colorBlendAttachmentState();

    //use the triangle layout we created
    pipelineBuilder.pipelineLayout = pipelineLayout;
    
    pipelineBuilder.depthStencil = init::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    //finally build the pipeline
    trianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);
    
    pipelineBuilder.shaderStages.clear();
    
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, coloredTriangleVertexShader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, coloredTriangleFragShader));
    
    coloredTrianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);
    
    //build the mesh pipeline

    VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();
    
    //clear the shader stages for the builder
    pipelineBuilder.shaderStages.clear();

    //compile mesh vertex shader
    loadShaderModule(shell_->explorer_.shader("TriangleMesh.vert.spv").c_str(), &meshVertexShader) ?
    shell_->log(Shell::LOG_INFO, "Triangle mesh vertex shader succesfully loaded") :
    shell_->log(Shell::LOG_ERROR, "Error when building the triangle mesh vertex shader module");

    //add the other shaders
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertexShader));

    //make sure that triangleFragShader is holding the compiled colored_triangle.frag
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, coloredTriangleFragShader));
    
    //we start from just the default empty pipeline layout info
    VkPipelineLayoutCreateInfo info = init::pipelineLayoutCreateInfo();
    
    //setup push constants
    VkPushConstantRange pushConstant;
    //this push constant range starts at the beginning
    pushConstant.offset = 0;
    //this push constant range takes up the size of a MeshPushConstants struct
    pushConstant.size = sizeof(MeshPushConstants);
    //this push constant range is accessible only in the vertex shader
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    info.pPushConstantRanges = &pushConstant;
    info.pushConstantRangeCount = 1;

    vkCreatePipelineLayout(device, &info, nullptr, &meshPipelineLayout) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Pipeline layout successfully created") :
    shell_->log(Shell::LOG_ERROR, "Pipeline layout failed to create!");
    
    pipelineBuilder.pipelineLayout = meshPipelineLayout;

    //build the mesh triangle pipeline
    meshPipeline = pipelineBuilder.buildPipeline(device, renderPass);

    //deleting all of the vulkan shaders
    vkDestroyShaderModule(device, meshVertexShader, nullptr);
    vkDestroyShaderModule(device, triangleVertexShader, nullptr);
    vkDestroyShaderModule(device, triangleFragShader, nullptr);
    vkDestroyShaderModule(device, coloredTriangleFragShader, nullptr);
    vkDestroyShaderModule(device, coloredTriangleVertexShader, nullptr);

    //adding the pipelines to the deletion queue
    mainDeletionQueue.push_function([=]() {
        vkDestroyPipeline(device, trianglePipeline, nullptr);
        vkDestroyPipeline(device, trianglePipeline, nullptr);
        vkDestroyPipeline(device, meshPipeline, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipelineLayout(device, meshPipelineLayout, nullptr);
    });
}

bool Daedalus::loadShaderModule(const char *filePath, VkShaderModule *shaderModule) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) { return false; }
    
    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //put file cursor at beggining
    file.seekg(0);

    //load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    //now that the file is loaded into the buffer, we can close it
    file.close();
    
    //create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;

    //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
    shaderModuleCreateInfo.codeSize = buffer.size() * sizeof(uint32_t);
    shaderModuleCreateInfo.pCode = buffer.data();

    //check that the creation goes well.
    VkShaderModule result;
    if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &result) != VK_SUCCESS) { return false; }
    *shaderModule = result;
    
    return true;
}

void Daedalus::loadMeshes() {
    //make the array 3 vertices long
    triangleMesh.vertices.resize(3);

    //vertex positions
    triangleMesh.vertices[0].position = { .5f, .5f, 0.0f };
    triangleMesh.vertices[1].position = {-.5f, .5f, 0.0f };
    triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

    triangleMesh.vertices[0].color = { 0.f, 0.f, 1.f};
    triangleMesh.vertices[1].color = { 0.f, 0.f, 1.f };
    triangleMesh.vertices[2].color = { 0.75f, 0.75f, 1.f };
    
    monkeyMesh.loadFromObj(shell_->explorer_.asset("Buddha.obj").c_str());

    uploadMesh(triangleMesh);
    uploadMesh(monkeyMesh);
}

void Daedalus::uploadMesh(Mesh &mesh) {
    //allocate vertex buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    //this is the total size, in bytes, of the buffer we are allocating
    bufferInfo.size = mesh.vertices.size() * sizeof(Vertex);
    //this buffer is going to be used as a Vertex Buffer
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;


    //let the VMA library know that this data should be writeable by CPU, but also readable by GPU
    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    //allocate the buffer
    vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
        &mesh.vertexBuffer.buffer,
        &mesh.vertexBuffer.allocation,
        nullptr) == VK_SUCCESS ?
    shell_->log(Shell::LOG_INFO, "Mesh buffer successfully allocated") :
    shell_->log(Shell::LOG_ERROR, "Mesh buffer failed to allocate!");

    //add the destruction of triangle mesh buffer to the deletion queue
    mainDeletionQueue.push_function([=]() {
        vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation);
    });
    
    //copy vertex data
    void* data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);

    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}
