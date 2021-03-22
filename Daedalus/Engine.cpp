#include "Engine.h"

#define VMA_IMPLEMENTATION
#include "MemoryAllocator.h"
#include "Types.h"
#include "Initializers.h"
#include "Bootstrap.h"
#include "Pipeline.h"

#include <algorithm>
#include <iostream>
#include <fstream>

// We want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state
#define VK_CHECK(x)                                                      \
    do                                                                   \
    {                                                                    \
        VkResult error = x;                                              \
        if (error)                                                       \
        {                                                                \
            std::cout <<"Detected Vulkan error: " << error << std::endl; \
            abort();                                                     \
        }                                                                \
    } while (0)

Engine::Engine(const std::vector<std::string> &args, void *caMetalLayer) : caMetalLayer(caMetalLayer) {
    settings.selectedShader = 0;
    
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
    }
}

void Engine::terminateSwapchain() {
    std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer) {
        vkDestroyFramebuffer(device, framebuffer, nullptr); });
    
    vkFreeCommandBuffers(device, commandPool, 1, &mainCommandBuffer);
    
    vkDestroyPipeline(device, trianglePipeline, nullptr);
    vkDestroyPipeline(device, coloredTrianglePipeline, nullptr);
    
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
    // Everything that happens in the world is updated, should be using ticks
}

void Engine::render() {
    //wait until the GPU has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(device, 1, &renderFence, VK_TRUE, 1000000000));
    VK_CHECK(vkResetFences(device, 1, &renderFence));
    
    //request image from the swapchain, one second timeout
    uint32_t swapchainImageIndex;
    VkResult swapchainStatus = vkAcquireNextImageKHR(device, swapchain, 1000000000, presentSemaphore, nullptr, &swapchainImageIndex);
    if (swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR) {
        updateSwapchain();
        return;
    } else if (swapchainStatus != VK_SUCCESS && swapchainStatus != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }
    
    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(mainCommandBuffer, 0));
    
    //naming it cmd for shorter writing
    VkCommandBuffer cmd = mainCommandBuffer;

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &commandBufferBeginInfo));
    
    //make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;
    float fadeRed = pow(2, cos(frameNumber / 25.f)) / 10.f;
    float fadeGreen = pow(2, cos(frameNumber / 20.f)) / 10.f;
    float fadeBlue = pow(2, sin(frameNumber / 25.f)) / 10.f;
    
    clearValue.color = { { fadeRed, fadeGreen, fadeBlue, 1.0f } };

    //start the main renderpass.
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.pNext = nullptr;

    rpInfo.renderPass = renderPass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = settings.windowExtent;
    rpInfo.framebuffer = framebuffers[swapchainImageIndex];

    //connect clear values
    rpInfo.clearValueCount = 1;
    rpInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    //once we start adding rendering commands, they will go here

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, settings.selectedShader == 0 ? trianglePipeline : coloredTrianglePipeline);
    vkCmdDraw(cmd, 3, 1, 0, 0);
    
    vkCmdEndRenderPass(cmd);
    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));
    
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
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, renderFence));
    
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
        updateSwapchain();
        return;
    }
    else if (swapchainStatus != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swapchain image!");
    }

    //increase the number of frames drawn
    frameNumber++;
}

void Engine::onKey(Key key) {
    switch (key) {
        case KEY_A:
            settings.selectedShader = settings.selectedShader == 0 ? 1 : 0;
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
            settings.selectedShader = settings.selectedShader == 0 ? 1 : 0;
            break;
    }
}

void Engine::initializeVulkan() {
    // Instance and debug messenger creation
    vkb::InstanceBuilder builder;
    
    auto instanceBuilder = builder.set_engine_name(settings.engineName.c_str())
        .set_engine_version(0, 1)
        .set_app_name(settings.applicationName.c_str())
        .set_app_version(0, 1)
        .require_api_version(1, 1, 0)
        .build();
    
    if (!instanceBuilder)
        std::cerr << "Failed to create Vulkan instance: " << instanceBuilder.error() << "\n";
    
    vkb::Instance vkbInstance = instanceBuilder.value();
    
    this->instance = vkbInstance.instance;
    this->debugMessenger = vkbInstance.debug_messenger;
    
    // Surface creation
    VkMetalSurfaceCreateInfoEXT surfaceInfo;
    surfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    surfaceInfo.pNext = nullptr;
    surfaceInfo.flags = 0;
    surfaceInfo.pLayer = caMetalLayer;
    VK_CHECK(vkCreateMetalSurfaceEXT(this->instance, &surfaceInfo, nullptr, &this->surface));
    
    // Physical device creation
    vkb::PhysicalDeviceSelector physicalDeviceSelector{vkbInstance};
    auto physicalDevice = physicalDeviceSelector
        .set_minimum_version(1, 1)
        .set_surface(this->surface)
        .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
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
}

void Engine::initializeCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = init::commandPoolCreateInfo(graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool));
    
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = init::commandBufferAllocateInfo(commandPool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &mainCommandBuffer));
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

    //we are going to create 1 subpass, which is the minimum you can do
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentReference;
    
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the color attachment to the info
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    //connect the subpass to the info
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
}

void Engine::initializeFramebuffers() {
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = renderPass;
    framebufferCreateInfo.attachmentCount = 1;
    framebufferCreateInfo.width = settings.windowExtent.width;
    framebufferCreateInfo.height = settings.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchain_imagecount = swapchainImages.size();
    framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

    //create framebuffers for each of the swapchain image views
    for (int i = 0; i < swapchain_imagecount; i++) {
        framebufferCreateInfo.pAttachments = &swapchainImageViews[i];
        VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));
    }
}

void Engine::initializeSyncStructures() {
    //create syncronization structures
        
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &renderFence));

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &presentSemaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphore));
}

void Engine::initializePipelines() {
    if (!isInitialized) {
        if (!loadShaderModule("ColoredTriangle.frag.spv", &coloredTriangleFragShader)) {
            std::cout << "Error when building the triangle fragment shader module" << "\n";
        } else { std::cout << "Colored triangle fragment shader succesfully loaded" << "\n"; }

        if (!loadShaderModule("ColoredTriangle.vert.spv", &coloredTriangleVertexShader)) {
            std::cout << "Error when building the triangle vertex shader module" << "\n";
        } else { std::cout << "Colored triangle vertex shader succesfully loaded" << "\n"; }
        
        if (!loadShaderModule("Triangle.frag.spv", &triangleFragShader)) {
            std::cout << "Error when building the triangle fragment shader module" << "\n";
        } else { std::cout << "Triangle fragment shader succesfully loaded" << "\n"; }

        if (!loadShaderModule("Triangle.vert.spv", &triangleVertexShader)) {
            std::cout << "Error when building the triangle vertex shader module" << "\n";
        } else { std::cout << "Triangle vertex shader succesfully loaded" << "\n"; }
    }
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = init::pipelineLayoutCreateInfo();
    
    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    
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

    //finally build the pipeline
    trianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);
    
    pipelineBuilder.shaderStages.clear();
    
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, coloredTriangleVertexShader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, coloredTriangleFragShader));
    
    coloredTrianglePipeline = pipelineBuilder.buildPipeline(device, renderPass);
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
