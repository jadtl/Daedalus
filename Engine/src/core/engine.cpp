#include "engine.h"

#define VMA_IMPLEMENTATION
#include "memory/allocator.h"
#include "renderer/types.h"
#include "renderer/init.h"
#include "renderer/bootstrap.h"
#include "renderer/pipeline.h"

#include <algorithm>
#include <iostream>
#include <fstream>

// We want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state
#define VK_CHECK(x)                                                       \
    do                                                                    \
    {                                                                     \
        VkResult error = x;                                               \
        if (error)                                                        \
        {                                                                 \
            std::cout << "Detected Vulkan error: " << error << std::endl; \
            abort();                                                      \
        }                                                                 \
    } while (0)

Engine::Engine(const std::vector<std::string> &args) : shell("../../")
{
    settings.selectedShader = 0;

    if (std::find(args.begin(), args.end(), "-validate") != args.end())
    {
        settings.validate = true;
    }
    if (std::find(args.begin(), args.end(), "-verbose") != args.end())
    {
        settings.validate = true;
        settings.verbose = true;
    }
}

Engine::~Engine()
{
    // Destroy context
    terminate();
}

void Engine::initialize()
{
    initializeWindow();

    initializeVulkan();

    initializeSwapchain();

    initializeCommands();

    initializeDefaultRenderPass();

    initializeFramebuffers();

    initializeSyncStructures();

    initializePipelines();

    loadMeshes();

    init_scene();

    isInitialized = true;
}

void Engine::terminate()
{
    if (isInitialized)
    {
        terminateSwapchain();

        // Terminate sync objects
        vkDestroySemaphore(device, present_semaphore, nullptr);
        vkDestroySemaphore(device, render_semaphore, nullptr);
        vkDestroyFence(device, render_fence, nullptr);

        vkDestroyCommandPool(device, command_pool, nullptr);

        vkDestroyDevice(device, nullptr);

        if (settings.validate)
            vkb::destroy_debug_utils_messenger(instance, debug_messenger);

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);

        vmaDestroyAllocator(allocator);
    }
}

void Engine::terminateSwapchain()
{
    std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer)
                  { vkDestroyFramebuffer(device, framebuffer, nullptr); });

    vkFreeCommandBuffers(device, command_pool, 1, &main_command_buffer);

    vkDestroyPipeline(device, triangle_pipeline, nullptr);
    vkDestroyPipeline(device, colored_triangle_pipeline, nullptr);
    vkDestroyPipeline(device, mesh_pipeline, nullptr);

    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);

    std::for_each(swapchain_image_views.begin(), swapchain_image_views.end(), [device = device](VkImageView imageView)
                  { vkDestroyImageView(device, imageView, nullptr); });

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Engine::updateSwapchain()
{
    vkDeviceWaitIdle(device);

    terminateSwapchain();

    initializeSwapchain();
    initializeDefaultRenderPass();
    initializePipelines();
    initializeFramebuffers();
    initializeCommands();
}

void Engine::update()
{
    // Everything that happens in the world is updated, should be using ticks
}

void Engine::render()
{
    //wait until the GPU has finished rendering the last frame. Timeout of 1 second
    VK_CHECK(vkWaitForFences(device, 1, &render_fence, VK_TRUE, 1000000000));
    VK_CHECK(vkResetFences(device, 1, &render_fence));

    //request image from the swapchain, one second timeout
    uint32_t swapchainImageIndex;
    VkResult swapchainStatus = vkAcquireNextImageKHR(device, swapchain, 1000000000, present_semaphore, nullptr, &swapchainImageIndex);
    if (swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR)
    {
        updateSwapchain();
        return;
    }
    else if (swapchainStatus != VK_SUCCESS && swapchainStatus != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(main_command_buffer, 0));

    //naming it cmd for shorter writing
    VkCommandBuffer cmd = main_command_buffer;

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;

    commandBufferBeginInfo.pInheritanceInfo = nullptr;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &commandBufferBeginInfo));

    //make a clear-color from frame number. This will flash with a 120*pi frame period.
    VkClearValue clearValue;

    float fadeBlue = abs(sin(frameNumber / 50.f)) / 7.5f;
    clearValue.color = {{0.f, 0.f, fadeBlue, 1.0f}};

    //clear depth at 1
    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;

    //start the main renderpass.
    //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
    VkRenderPassBeginInfo renderPassInfo = init::renderPassBeginInfo(render_pass, settings.windowExtent, framebuffers[swapchainImageIndex]);

    //connect clear values
    renderPassInfo.clearValueCount = 2;

    VkClearValue clearValues[] = {clearValue, depthClear};

    renderPassInfo.pClearValues = &clearValues[0];

    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    draw_objects(cmd, renderables.data(), renderables.size());

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
    submit.pWaitSemaphores = &present_semaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &render_semaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;

    //submit command buffer to the queue and execute it.
    // _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit, render_fence));

    // this will put the image we just rendered into the visible window.
    // we want to wait on the _renderSemaphore for that,
    // as it's necessary that drawing commands have finished before the image is displayed to the user
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;

    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &render_semaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &swapchainImageIndex;

    swapchainStatus = vkQueuePresentKHR(graphics_queue, &presentInfo);
    if (swapchainStatus == VK_SUBOPTIMAL_KHR || swapchainStatus == VK_ERROR_OUT_OF_DATE_KHR)
    {
        updateSwapchain();
        return;
    }
    else if (swapchainStatus != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swapchain image!");
    }

    //increase the number of frames drawn
    frameNumber++;
}

void Engine::onKey(Key key)
{
    switch (key)
    {
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

void Engine::initializeWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    this->window = glfwCreateWindow(800, 600, "Daedalus", NULL, NULL);
}

void Engine::initializeVulkan()
{
    // Instance and debug messenger creation
    vkb::InstanceBuilder builder;

    auto instanceBuilder = builder.set_engine_name(settings.engineName.c_str())
                               .set_engine_version(0, 1)
                               .set_app_name(settings.applicationName.c_str())
                               .set_app_version(0, 1)
                               .request_validation_layers()
                               .add_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
                               .require_api_version(1, 1, 0)
                               .build();

    if (!instanceBuilder)
        std::cerr << "Failed to create Vulkan instance: " << instanceBuilder.error() << "\n";

    vkb::Instance vkbInstance = instanceBuilder.value();

    this->instance = vkbInstance.instance;
    this->debug_messenger = vkbInstance.debug_messenger;

    // Surface creation
    if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface))
    {
        throw std::runtime_error("Failed to create window surface!");
    }

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
    this->physical_device = physicalDevice.value().physical_device;

    graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphics_queue_family = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo info{};
    info.physicalDevice = this->physical_device;
    info.device = this->device;
    info.instance = this->instance;
    vmaCreateAllocator(&info, &this->allocator);

    loadMeshes();
}

void Engine::initializeSwapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{physical_device, device, surface};

    vkb::Swapchain vkbSwapchain = swapchainBuilder
                                      .use_default_format_selection()
                                      .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                      .build()
                                      .value();

    swapchain = vkbSwapchain.swapchain;
    swapchain_images = vkbSwapchain.get_images().value();
    swapchain_image_views = vkbSwapchain.get_image_views().value();
    settings.windowExtent = vkbSwapchain.extent;
    swapchain_image_format = vkbSwapchain.image_format;

    //depth image size will match the window
    VkExtent3D depthImageExtent = {
        settings.windowExtent.width,
        settings.windowExtent.height,
        1};

    //hardcoding the depth format to 32 bit float
    depth_format = VK_FORMAT_D32_SFLOAT;

    //the depth image will be an image with the format we selected and Depth Attachment usage flag
    VkImageCreateInfo dimg_info = init::imageCreateInfo(depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

    //for the depth image, we want to allocate it from GPU local memory
    VmaAllocationCreateInfo dimg_allocinfo = {};
    dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    vmaCreateImage(allocator, &dimg_info, &dimg_allocinfo, &depth_image.image, &depth_image.allocation, nullptr);

    //build an image-view for the depth image to use for rendering
    VkImageViewCreateInfo dview_info = init::imageViewCreateInfo(depth_format, depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

    VK_CHECK(vkCreateImageView(device, &dview_info, nullptr, &depth_image_view));

    //add to deletion queues
    main_deletion_queue.push_function([=]()
                                      {
                                          vkDestroyImageView(device, depth_image_view, nullptr);
                                          vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);
                                      });
}

void Engine::initializeCommands()
{
    VkCommandPoolCreateInfo commandPoolInfo = init::commandPoolCreateInfo(graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &command_pool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = init::commandBufferAllocateInfo(command_pool, 1);

    VK_CHECK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &main_command_buffer));
}

void Engine::initializeDefaultRenderPass()
{
    // the renderpass will use this color attachment.
    VkAttachmentDescription colorAttachment = {};
    //the attachment will have the format needed by the swapchain
    colorAttachment.format = swapchain_image_format;
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
    depthAttachment.format = depth_format;
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
    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    //connect the color attachment to the info
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = &attachments[0];
    //connect the subpass to the info
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass));
}

void Engine::initializeFramebuffers()
{
    //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
    VkFramebufferCreateInfo framebufferCreateInfo = {};
    framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferCreateInfo.pNext = nullptr;

    framebufferCreateInfo.renderPass = render_pass;
    framebufferCreateInfo.width = settings.windowExtent.width;
    framebufferCreateInfo.height = settings.windowExtent.height;
    framebufferCreateInfo.layers = 1;

    //grab how many images we have in the swapchain
    const uint32_t swapchainImagecount = swapchain_images.size();
    framebuffers = std::vector<VkFramebuffer>(swapchainImagecount);

    //create framebuffers for each of the swapchain image views
    for (int i = 0; i < swapchainImagecount; i++)
    {
        VkImageView attachments[2];
        attachments[0] = swapchain_image_views[i];
        attachments[1] = depth_image_view;

        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.attachmentCount = 2;

        VK_CHECK(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffers[i]));
    }
}

void Engine::initializeSyncStructures()
{
    //create syncronization structures

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &render_fence));

    //for the semaphores we don't need any flags
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;
    semaphoreCreateInfo.flags = 0;

    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &present_semaphore));
    VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &render_semaphore));
}

void Engine::initializePipelines()
{
    if (!loadShaderModule(shell.shader("ColoredTriangle.frag.spv").c_str(), &colored_triangle_frag_shader))
    {
        std::cout << "Error when building the triangle fragment shader module"
                  << "\n";
    }
    else
    {
        std::cout << "Colored triangle fragment shader succesfully loaded"
                  << "\n";
    }

    if (!loadShaderModule(shell.shader("ColoredTriangle.vert.spv").c_str(), &colored_triangle_vertex_shader))
    {
        std::cout << "Error when building the triangle vertex shader module"
                  << "\n";
    }
    else
    {
        std::cout << "Colored triangle vertex shader succesfully loaded"
                  << "\n";
    }

    if (!loadShaderModule(shell.shader("Triangle.frag.spv").c_str(), &triangle_frag_shader))
    {
        std::cout << "Error when building the triangle fragment shader module"
                  << "\n";
    }
    else
    {
        std::cout << "Triangle fragment shader succesfully loaded"
                  << "\n";
    }

    if (!loadShaderModule(shell.shader("Triangle.vert.spv").c_str(), &triangle_vertex_shader))
    {
        std::cout << "Error when building the triangle vertex shader module"
                  << "\n";
    }
    else
    {
        std::cout << "Triangle vertex shader succesfully loaded"
                  << "\n";
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = init::pipelineLayoutCreateInfo();

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline_layout));

    //build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
    PipelineBuilder pipelineBuilder;

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangle_vertex_shader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));

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
    pipelineBuilder.pipelineLayout = pipeline_layout;

    pipelineBuilder.depthStencil = init::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    //finally build the pipeline
    triangle_pipeline = pipelineBuilder.buildPipeline(device, render_pass);

    pipelineBuilder.shaderStages.clear();

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, colored_triangle_vertex_shader));

    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colored_triangle_frag_shader));

    colored_triangle_pipeline = pipelineBuilder.buildPipeline(device, render_pass);

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
    if (!loadShaderModule(shell.shader("TriangleMesh.vert.spv").c_str(), &mesh_vertex_shader))
    {
        std::cout << "Error when building the triangle mesh vertex shader module" << std::endl;
    }
    else
    {
        std::cout << "Triangle mesh vertex shader succesfully loaded" << std::endl;
    }

    //add the other shaders
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, mesh_vertex_shader));

    //make sure that triangleFragShader is holding the compiled colored_triangle.frag
    pipelineBuilder.shaderStages.push_back(
        init::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colored_triangle_frag_shader));

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

    VK_CHECK(vkCreatePipelineLayout(device, &info, nullptr, &mesh_pipeline_layout));

    pipelineBuilder.pipelineLayout = mesh_pipeline_layout;

    //build the mesh pipeline
    mesh_pipeline = pipelineBuilder.buildPipeline(device, render_pass);

    create_material(mesh_pipeline, mesh_pipeline_layout, "defaultmesh");

    //deleting all of the vulkan shaders
    vkDestroyShaderModule(device, mesh_vertex_shader, nullptr);
    vkDestroyShaderModule(device, triangle_vertex_shader, nullptr);
    vkDestroyShaderModule(device, triangle_frag_shader, nullptr);
    vkDestroyShaderModule(device, colored_triangle_frag_shader, nullptr);
    vkDestroyShaderModule(device, colored_triangle_vertex_shader, nullptr);

    //adding the pipelines to the deletion queue
    main_deletion_queue.push_function([=]()
                                      {
                                          vkDestroyPipeline(device, triangle_pipeline, nullptr);
                                          vkDestroyPipeline(device, triangle_pipeline, nullptr);
                                          vkDestroyPipeline(device, mesh_pipeline, nullptr);

                                          vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
                                          vkDestroyPipelineLayout(device, mesh_pipeline_layout, nullptr);
                                      });
}

bool Engine::loadShaderModule(const char *filePath, VkShaderModule *shaderModule)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        return false;
    }

    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //put file cursor at beggining
    file.seekg(0);

    //load the entire file into the buffer
    file.read((char *)buffer.data(), fileSize);

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
    if (vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &result) != VK_SUCCESS)
    {
        return false;
    }
    *shaderModule = result;

    return true;
}

void Engine::loadMeshes()
{
    //make the array 3 vertices long
    triangle_mesh.vertices.resize(3);

    //vertex positions
    triangle_mesh.vertices[0].position = {.5f, .5f, 0.0f};
    triangle_mesh.vertices[1].position = {-.5f, .5f, 0.0f};
    triangle_mesh.vertices[2].position = {0.f, -1.f, 0.0f};

    triangle_mesh.vertices[0].color = {0.f, 0.f, 1.f};
    triangle_mesh.vertices[1].color = {0.f, 0.f, 1.f};
    triangle_mesh.vertices[2].color = {0.75f, 0.75f, 1.f};

    monkey_mesh.loadFromObj(shell.asset("MonkeySmooth.obj").c_str());

    uploadMesh(triangle_mesh);
    uploadMesh(monkey_mesh);

    this->meshes["triangle"] = triangle_mesh;
    this->meshes["monkey"] = monkey_mesh;
}

void Engine::uploadMesh(Mesh &mesh)
{
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
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
                             &mesh.vertexBuffer.buffer,
                             &mesh.vertexBuffer.allocation,
                             nullptr));

    //add the destruction of triangle mesh buffer to the deletion queue
    main_deletion_queue.push_function([=]()
                                      { vmaDestroyBuffer(allocator, mesh.vertexBuffer.buffer, mesh.vertexBuffer.allocation); });

    //copy vertex data
    void *data;
    vmaMapMemory(allocator, mesh.vertexBuffer.allocation, &data);

    memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

    vmaUnmapMemory(allocator, mesh.vertexBuffer.allocation);
}

Material *Engine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name)
{
    Material mat;
    mat.pipeline = pipeline;
    mat.pipeline_layout = layout;
    this->materials[name] = mat;
    return &this->materials[name];
}

Material *Engine::get_material(const std::string &name)
{
    //search for the object, and return nullptr if not found
    auto it = this->materials.find(name);
    if (it == this->materials.end())
    {
        return nullptr;
    }
    else
    {
        return &(*it).second;
    }
}

Mesh *Engine::get_mesh(const std::string &name)
{
    auto it = this->meshes.find(name);
    if (it == this->meshes.end())
    {
        return nullptr;
    }
    else
    {
        return &(*it).second;
    }
}

void Engine::draw_objects(VkCommandBuffer cmd, RenderObject *first, int count)
{
    //make a model view matrix for rendering the object
    //camera view
    glm::vec3 camPos = {0.f, -6.f, -10.f};

    glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
    //camera projection
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;

    Mesh *lastMesh = nullptr;
    Material *lastMaterial = nullptr;
    for (int i = 0; i < count; i++)
    {
        RenderObject &object = first[i];

        //only bind the pipeline if it doesn't match with the already bound one
        if (object.material != lastMaterial)
        {

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;
        }

        glm::mat4 model = object.transform_matrix;
        //final render matrix, that we are calculating on the cpu
        glm::mat4 mesh_matrix = projection * view * model;

        MeshPushConstants constants;
        constants.render_matrix = mesh_matrix;

        //upload the mesh to the GPU via push constants
        vkCmdPushConstants(cmd, object.material->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

        //only bind the mesh if it's a different one from last bind
        if (object.mesh != lastMesh)
        {
            //bind the mesh vertex buffer with offset 0
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertexBuffer.buffer, &offset);
            lastMesh = object.mesh;
        }
        //we can now draw
        vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
    }
}

void Engine::init_scene()
{
    RenderObject monkey;
    monkey.mesh = get_mesh("monkey");
    monkey.material = get_material("defaultmesh");
    monkey.transform_matrix = glm::mat4{1.0f};

    this->renderables.push_back(monkey);

    for (int x = -20; x <= 20; x++)
    {
        for (int y = -20; y <= 20; y++)
        {
            RenderObject tri;
            tri.mesh = get_mesh("triangle");
            tri.material = get_material("defaultmesh");
            glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3(x, 0, y));
            glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(0.2, 0.2, 0.2));
            tri.transform_matrix = translation * scale;

            this->renderables.push_back(tri);
        }
    }
}
