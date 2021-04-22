#include "Engine.h"

#include <glm/gtx/transform.hpp>

#define VMA_IMPLEMENTATION
#include "MemoryAllocator.h"
#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"
#include "Pipeline.h"

#include <algorithm>
#include <iostream>
#include <fstream>

Engine::Engine(const std::vector<std::string> &args, void *windowHandle) : windowHandle(windowHandle), console() {
    settings.engineName = "Daedalus [Vulkan]";
    settings.applicationName = "Unnamed Work [Vulkan]";
    
    if (std::find(args.begin(), args.end(), "-validate") != args.end()) { settings.validate = true; }
    if (std::find(args.begin(), args.end(), "-verbose") != args.end()) { settings.validate = true; settings.verbose = true; }
}

Engine::~Engine() {
    // Destroy context
    terminate();
}

void Engine::initialize() {
    initializeVulkan();
    
    initializeSwapchain();
    
    initializeCommands();
    
    initializeDefaultRenderPass();
    
    initializeFramebuffers();
    
    initializeSyncStructures();
    
    initializePipelines();
    
    isInitialized = true;
}

void Engine::terminate() {
    if (isInitialized) {
        mainDeletionQueue.flush();
        
        terminateSwapchain();
        
        // Terminate sync objects
        vkDestroySemaphore(device, presentSemaphore, nullptr);
        vkDestroySemaphore(device, renderSemaphore, nullptr);
        vkDestroyFence(device, renderFence, nullptr);
        
        vkDestroyCommandPool(device, commandPool, nullptr);
        
        vkDestroyDevice(device, nullptr);
        
        if (settings.validate) vkb::destroy_debug_utils_messenger(instance, debugMessenger);
        
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        
        vmaDestroyAllocator(allocator);
    }
}

void Engine::terminateSwapchain() {
    std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr); });
    
    vkFreeCommandBuffers(device, commandPool, 1, &mainCommandBuffer);
    
    vkDestroyPipeline(device, pipeline, nullptr);
    
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    
    std::for_each(swapchainImageViews.begin(), swapchainImageViews.end(), [device = device](VkImageView imageView) { vkDestroyImageView(device, imageView, nullptr); });
    
    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Engine::updateSwapchain() {
    vkDeviceWaitIdle(device);
    
    terminateSwapchain();
    
    initializeSwapchain();
    initializeDefaultRenderPass();
    initializePipelines();
    initializeFramebuffers();
    initializeCommands();
}

void Engine::update() {
    //everything that happens in the world is updated, should be using ticks
}

void Engine::render() {
    //wait until the GPU has finished rendering the last frame. Timeout of 1 second
    check("Waiting for fences", vkWaitForFences(device, 1, &renderFence, VK_TRUE, 1000000000));
    check("Fences reset", vkResetFences(device, 1, &renderFence));
    
    //request image from the swapchain, one second timeout
    uint32_t swapchainImageIndex;

    VkResult swapchainStatus = vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, NULL, &swapchainImageIndex);
    if (swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR) {
        console.log(Console::LOG_INFO, "Updating out-of-date swapchain...");
        updateSwapchain();
        return;
    } else if (swapchainStatus != VK_SUCCESS && swapchainStatus != VK_SUBOPTIMAL_KHR)
        console.log(Console::LOG_FATAL, "Failure: Acquiring swapchain image");
    
    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
    check("Command buffer reset", vkResetCommandBuffer(mainCommandBuffer, 0));
    
    //naming it cmd for shorter writing
    VkCommandBuffer cmd = mainCommandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    check("Begin command buffer", vkBeginCommandBuffer(cmd, &commandBufferBeginInfo));
    
    //make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    
    float fadeBlue = abs(sin(frameNumber / 25.f)) / 7.5f;
    clearValue.color = { { 0.f, 0.f, fadeBlue, 1.0f } };
    
    //clear depth at 1
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    //start the main renderpass.
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo renderPassInfo = init::renderPassBeginInfo(renderPass, settings.windowExtent, framebuffers[swapchainImageIndex]);

    //connect clear values
    renderPassInfo.clearValueCount = 2;
    
    VkClearValue clearValues[] = { clearValue, depthClear };

    renderPassInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    //drawing start
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    //make a model view matrix for rendering the object
    //camera position
    glm::vec3 cameraPosition = { 0.f, -2.f, -15.f };

    glm::mat4 view = glm::translate(glm::mat4(1.f), cameraPosition);
    //camera projection
    glm::mat4 projection = glm::perspective(glm::radians(45.f), (float)(settings.windowExtent.width / settings.windowExtent.height), 0.1f, 100.0f);
    projection[1][1] *= -1;
    //model rotation
    glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::radians(frameNumber * 1.f), glm::vec3(0, 1, 0));

    //calculate final mesh matrix
    glm::mat4 meshMatrix = projection * view * model;

    MeshPushConstants constants;
    constants.renderMatrix = meshMatrix;

    //upload the matrix to the GPU via pushconstants
    vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
    
    //bind the mesh vertex buffer with offset 0
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &monkeyMesh.vertexBuffer.buffer, &offset);

    //we can now draw the mesh
    vkCmdDraw(cmd, monkeyMesh.vertices.size(), 1, 0, 0);
    
    vkCmdEndRenderPass(cmd);
    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    check("End command buffer", vkEndCommandBuffer(cmd));
    
    //prepare the submission to the queue.
    //we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
    //we will signal the _renderSemaphore, to signal that rendering has finished

    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    check("Queue submission", vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));
    
    // this will put the image we just rendered into the visible window.
    // we want to wait on the _renderSemaphore for that,
    // as it's necessary that drawing commands have finished before the image is displayed to the user
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    swapchainStatus = vkQueuePresentKHR(graphicsQueue, &presentInfo);
    if (swapchainStatus == VK_SUBOPTIMAL_KHR || swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR) {
        console.log(Console::LOG_INFO, "Updating out-of-date swapchain...");
        updateSwapchain();
        return;
    }
    else if (swapchainStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to present swapchain image!");

    // Increase the number of frames drawn
    frameNumber++;
}

void Engine::onKey(Key key) {
    switch (key) {
        case KEY_A:
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
            break;
    }
}

void Engine::initializeVulkan() {
    // Instance and debug messenger creation
    vkb::InstanceBuilder builder;
    
    auto instanceBuilder = (settings.validate) ?
        builder
        .set_engine_name(settings.engineName)
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName)
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0)
        .request_validation_layers()
        .use_default_debug_messenger()
        .build()
        :
        builder
        .set_engine_name(settings.engineName)
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName)
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0)
        .build();
    
    if (!instanceBuilder)
        console.log(Console::LOG_ERROR, "Failed to create Vulkan instance: " + instanceBuilder.error().message() + "\n");
    
    vkb::Instance vkbInstance = instanceBuilder.value();
    
    this->instance = vkbInstance.instance;
    this->debugMessenger = vkbInstance.debug_messenger;

#if defined(_WIN32)
    
#elif defined(__APPLE__)
    // Surface creation
    VkMetalSurfaceCreateInfoEXT surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.pLayer = windowHandle;
    check("Metal surface creation", vkCreateMetalSurfaceEXT(this->instance, &surfaceInfo, nullptr, &this->surface));
#endif
    
    // Physical device creation
    vkb::PhysicalDeviceSelector physicalDeviceSelector{vkbInstance};
    auto physicalDevice = physicalDeviceSelector
        .set_minimum_version(1, 1)
        .set_surface(this->surface)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
        .add_desired_extension("VK_KHR_portability_subset")
        .select();
        
    if (!physicalDevice)
        std::cerr << "Failed to select Vulkan physical device: " << physicalDevice.error().message() << "\n";
    
    // Device creation
    vkb::DeviceBuilder deviceBuilder{physicalDevice.value()};
    vkb::Device vkbDevice = deviceBuilder.build().value();
    
    this->device = vkbDevice.device;
    this->physicalDevice = physicalDevice.value().physical_device;
    
    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    
    VmaAllocatorCreateInfo info{};
    info.physicalDevice = this->physicalDevice;
    info.device = this->device;
    info.instance = this->instance;
    vmaCreateAllocator(&info, &this->allocator);
    
    loadMeshes();
}

void Engine::initializeSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{physicalDevice, device, surface};
    
    vkb::Swapchain vkbSwapchain = swapchainBuilder
        .use_default_format_selection()
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .build()
        .value();
    
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
    settings.windowExtent = vkbSwapchain.extent;
    swapchainImageFormat = vkbSwapchain.image_format;
    
    //depth image size will match the window
    VkExtent3D depthImageExtent = {
        settings.windowExtent.width,
        settings.windowExtent.height,
        1
    };

    //hardcoding the depth format to 32 bit float
    depthFormat = VK_FORMAT_D32_SFLOAT;

    //the depth image will be an image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo dimg_info = init::imageCreateInfo(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    //for the depth image, we want to allocate it from GPU local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(allocator, &dimg_info, &dimg_allocinfo, &depthImage.image, &depthImage.allocation, nullptr);

    //build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo dview_info = init::imageViewCreateInfo(depthFormat, depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    check("Image view creation", vkCreateImageView(device, &dview_info, nullptr, &depthImageView));

    //add to deletion queues
    mainDeletionQueue.push_function([=]() {
        vkDestroyImageView(device, depthImageView, nullptr);
        vmaDestroyImage(allocator, depthImage.image, depthImage.allocation);
    });
}

void Engine::initializeCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = init::commandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    check("Command pool creation", vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));
    
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = init::commandBufferAllocateInfo(commandPool, 1);

    check("Command buffers allocation", vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &mainCommandBuffer));
}

void Engine::initializeDefaultRenderPass() {
    // the renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment = {};
    //the attachment will have the format needed by the swapchain
    colorAttachment.format = swapchainImageFormat;
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

    check("Render pass creation", vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void Engine::initializeFramebuffers() {
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.width = settings.windowExtent.width;
    framebufferCreateInfo.height = settings.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchainImagecount = swapchainImages.size();
    framebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

    //create framebuffers for each of the swapchain image views
    for (int i = 0; i < swapchainImagecount; i++) {
        VkImageView attachments[2];
        attachments[0] = swapchainImageViews[i];
        attachments[1] = depthImageView;

        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.attachmentCount = 2;
        
        check("Framebuffer creation", vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));
    }
}

void Engine::initializeSyncStructures() {
    //create syncronization structures
        
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    check("Fence creation", vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    check("Present semaphore creation", vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
    check("Render sempahore creation", vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));
}

void Engine::initializePipelines() {
    check("Fragment shader loading", loadShaderModule(explorer->shader("ColoredTriangle.frag.spv").c_str(), &fragmentShader));
    
    //compile mesh vertex shader
    check("Vertex shader loading", loadShaderModule(explorer->shader("TriangleMesh.vert.spv").c_str(), &vertexShader));

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = init::pipelineLayoutCreateInfo();
    
    //setup push constants
    VkPushConstantRange pushConstant;
    //this push constant range starts at the beginning
    pushConstant.offset = 0;
    //this push constant range takes up the size of a MeshPushConstants struct
    pushConstant.size = sizeof(MeshPushConstants);
    //this push constant range is accessible only in the vertex shader
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    
    check("Pipeline layout creation", vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    
    //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
    PipelineBuilder pipelineBuilder;

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShader));

    //vertex input controls how to read vertices from vertex buffers. We aren't using it yet
    pipelineBuilder.vertexInputInfo = init::vertexInputStateCreateInfo();
    
    //input assembly is the configuration for drawing triangle lists, strips, or individual points.
    //we are just going to draw triangle list
    pipelineBuilder.inputAssembly = init::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    //build viewport and scissor from the swapchain extents
    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float)settings.windowExtent.width;
    pipelineBuilder.viewport.height = (float)settings.windowExtent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;
    
    pipelineBuilder.scissor.offset = {0, 0};
    pipelineBuilder.scissor.extent = settings.windowExtent;

    //configure the rasterizer to draw filled triangles
    pipelineBuilder.rasterizer = init::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    //we don't use multisampling, so just run the default one
    pipelineBuilder.multisampling = init::multisamplingStateCreateInfo();

    //a single blend attachment with no blending and writing to RGBA
    pipelineBuilder.colorBlendAttachment = init::colorBlendAttachmentState();

    //use the triangle layout we created
    pipelineBuilder.pipelineLayout = pipelineLayout;
    
    pipelineBuilder.depthStencil = init::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();
    
    pipelineBuilder.pipelineLayout = pipelineLayout;

    //build the mesh triangle pipeline
    pipeline = pipelineBuilder.buildPipeline(device, renderPass);

    //deleting all of the vulkan shaders
    vkDestroyShaderModule(device, vertexShader, nullptr);
    vkDestroyShaderModule(device, fragmentShader, nullptr);

    //adding the pipelines to the deletion queue
    mainDeletionQueue.push_function([=]() {
        vkDestroyPipeline(device, pipeline, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    });
}

bool Engine::loadShaderModule(const char *filePath, VkShaderModule *shaderModule) {
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

void Engine::loadMeshes() {
    //make the array 3 vertices long
    triangleMesh.vertices.resize(3);

    //vertex positions
    triangleMesh.vertices[0].position = { .5f, .5f, 0.0f };
    triangleMesh.vertices[1].position = {-.5f, .5f, 0.0f };
    triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

    triangleMesh.vertices[0].color = { 0.f, 0.f, 1.f};
    triangleMesh.vertices[1].color = { 0.f, 0.f, 1.f };
    triangleMesh.vertices[2].color = { 0.75f, 0.75f, 1.f };
    
    monkeyMesh.loadFromObj(explorer->asset("Buddha.obj").c_str());

    uploadMesh(triangleMesh);
    uploadMesh(monkeyMesh);
}

void Engine::uploadMesh(Mesh &mesh) {
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
    check("Buffer allocation", vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
        &mesh.vertexBuffer.buffer,
        &mesh.vertexBuffer.allocation,
        nullptr));

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

void Engine::check(std::string action, bool result) {
    if (result) {
        if (settings.verbose)
            console.log(Console::LOG_VERBOSE, "Success: " + action);
    }
    else
        console.log(Console::LOG_ERROR, "Failure:" + action);
}

void Engine::check(std::string action, VkResult result) {
    if (result == VK_SUCCESS) {
        if (settings.verbose)
            console.log(Console::LOG_VERBOSE, "Success: " + action);
    }
    else
        console.log(Console::LOG_ERROR, "Failure:" + action);
}
