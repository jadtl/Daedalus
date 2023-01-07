#include <core/engine.h>
#include <core/log.h>

#include <renderer/types.h>
#include <renderer/init.h>
#include <renderer/pipeline.h>

#include <glm/gtc/type_ptr.hpp>

#include <VkBootstrap.h>
#define VMA_IMPLEMENTATION
#include <memory/allocator.h>

#include <algorithm>
#include <iostream>
#include <fstream>

namespace ddls {
    // We want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state
#define VK_CHECK(x)                                                       \
    do                                                                    \
    {                                                                     \
        VkResult error = x;                                               \
        if (error)                                                        \
        {                                                                 \
            Log::Error("Detected Vulkan error: ", error); \
            abort();                                                      \
        }                                                                 \
    } while (0)

    Engine::Engine(const std::vector<std::string>& args)
    {
        settings.selected_shader = 0;

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

    VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data)
    {
        switch (message_severity)
        {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Log::Error(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Log::Warning(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Log::Info(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Log::Debug(callback_data->pMessage);
            break;
        }
        return VK_FALSE;
    }

    void Engine::print_welcome() {
        Log::Info(fmt::format("Daedalus Engine v{}.{}.{}", DDLS_VERSION_MAJOR, DDLS_VERSION_MINOR, DDLS_VERSION_PATCH)
        );
    }

    void Engine::initialize()
    {   
        Log::OpenLog("Daedalus.log");
        Log::Info("Initializing engine...");
        
        initialize_window();
        initialize_vulkan();
        initialize_swapchain();
        initialize_commands();
        initialize_default_renderpass();
        initialize_framebuffers();
        initialize_sync_structures();
        initialize_pipelines();

        load_meshes();
        init_scene();

        is_initialized = true;
        print_welcome();
    }

    void Engine::terminate()
    {
        if (is_initialized)
        {
            vkDeviceWaitIdle(device);

            terminate_swapchain();

            main_deletion_queue.flush();

            vkDestroySurfaceKHR(instance, surface, nullptr);

            vkDestroyDevice(device, nullptr);
            vkb::destroy_debug_utils_messenger(instance, debug_messenger);
            vkDestroyInstance(instance, nullptr);

            glfwDestroyWindow(window);

            Log::Info("Closing log and shutting down...");
            Log::CloseLog();
        }
    }

    void Engine::terminate_swapchain()
    {
        std::for_each(framebuffers.begin(), framebuffers.end(), [device = device](VkFramebuffer framebuffer)
            { vkDestroyFramebuffer(device, framebuffer, nullptr); });

        vkFreeCommandBuffers(device, command_pool, 1, &main_command_buffer);

        vkDestroyCommandPool(device, command_pool, nullptr);

        vkDestroyPipeline(device, triangle_pipeline, nullptr);
        vkDestroyPipeline(device, colored_triangle_pipeline, nullptr);
        vkDestroyPipeline(device, mesh_pipeline, nullptr);

        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        vkDestroyPipelineLayout(device, mesh_pipeline_layout, nullptr);

        vkDestroyRenderPass(device, render_pass, nullptr);

        vkDestroyImageView(device, depth_image_view, nullptr);
        vmaDestroyImage(allocator, depth_image.image, depth_image.allocation);

        std::for_each(swapchain_image_views.begin(), swapchain_image_views.end(), [device = device](VkImageView image_view)
            { vkDestroyImageView(device, image_view, nullptr); });

        vkDestroySwapchainKHR(device, swapchain, nullptr);
    }

    void Engine::update_swapchain()
    {
        vkDeviceWaitIdle(device);

        terminate_swapchain();

        initialize_swapchain();
        initialize_default_renderpass();
        initialize_pipelines();
        initialize_framebuffers();
        initialize_commands();
    }

    void Engine::update()
    {
        // Everything that happens in the world is updated, should be using ticks
    }

    void Engine::render()
    {
        // Wait until the GPU has finished rendering the last frame. Timeout of 1 second
        VK_CHECK(vkWaitForFences(device, 1, &render_fence, VK_TRUE, 1000000000));
        VK_CHECK(vkResetFences(device, 1, &render_fence));

        // Request image from the swapchain, one second timeout
        uint32_t swapchain_image_index;
        VkResult swapchain_status = vkAcquireNextImageKHR(device, swapchain, 1000000000, present_semaphore, nullptr, &swapchain_image_index);
        if (swapchain_status == VK_ERROR_OUT_OF_DATE_KHR)
        {
            update_swapchain();
            return;
        }
        else if (swapchain_status != VK_SUCCESS && swapchain_status != VK_SUBOPTIMAL_KHR)
            Log::Error("Failed to acquire swapchain image!");
        
        // Now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
        VK_CHECK(vkResetCommandBuffer(main_command_buffer, 0));

        // Naming it cmd for shorter writing
        VkCommandBuffer cmd = main_command_buffer;

        // Begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
        VkCommandBufferBeginInfo command_buffer_begin_info = {};
        command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.pNext = nullptr;

        command_buffer_begin_info.pInheritanceInfo = nullptr;
        command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(cmd, &command_buffer_begin_info));

        // Make a clear-color from frame number. This will flash with a 120*pi frame period.
        VkClearValue clear_value;

        float fadeBlue = fabs(sin((f32)frame_number / 50.f)) / 7.5f;
        clear_value.color = { {0.f, 0.f, fadeBlue, 1.0f} };

        // Clear depth at 1
        VkClearValue depth_clear;
        depth_clear.depthStencil.depth = 1.f;

        // Start the main renderpass.
        // We will use the clear color from above, and the framebuffer of the index the swapchain gave us
        VkRenderPassBeginInfo render_pass_info = init::render_pass_begin_info(render_pass, settings.window_extent, framebuffers[swapchain_image_index]);

        // Connect clear values
        render_pass_info.clearValueCount = 2;

        VkClearValue clearValues[] = { clear_value, depth_clear };

        render_pass_info.pClearValues = &clearValues[0];

        vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        draw_objects(cmd, renderables.data(), renderables.size());

        vkCmdEndRenderPass(cmd);

        // Finalize the command buffer (we can no longer add commands, but it can now be executed)
        VK_CHECK(vkEndCommandBuffer(cmd));

        // Prepare the submission to the queue.
        // We want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
        // We will signal the _renderSemaphore, to signal that rendering has finished
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submit_info.pWaitDstStageMask = &waitStage;

        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &present_semaphore;

        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_semaphore;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        // Submit command buffer to the queue and execute it.
        // render_fence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit(graphics_queue, 1, &submit_info, render_fence));

        // This will put the image we just rendered into the visible window.
        // We want to wait on the _renderSemaphore for that,
        // as it's necessary that drawing commands have finished before the image is displayed to the user
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.pNext = nullptr;

        present_info.pSwapchains = &swapchain;
        present_info.swapchainCount = 1;

        present_info.pWaitSemaphores = &render_semaphore;
        present_info.waitSemaphoreCount = 1;

        present_info.pImageIndices = &swapchain_image_index;

        swapchain_status = vkQueuePresentKHR(graphics_queue, &present_info);
        if (swapchain_status == VK_SUBOPTIMAL_KHR || swapchain_status == VK_ERROR_OUT_OF_DATE_KHR)
        {
            update_swapchain();
            return;
        }
        else if (swapchain_status != VK_SUCCESS)
            Log::Error("Failed to present swapchain image!");
        
        // Increase the number of frames drawn
        frame_number++;
    }

    void Engine::on_key(Key key)
    {
        switch (key)
        {
        case KEY_A:
            settings.selected_shader = settings.selected_shader == 0 ? 1 : 0;
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
            settings.selected_shader = settings.selected_shader == 0 ? 1 : 0;
            break;
        }
    }

    void Engine::initialize_window()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        this->window = glfwCreateWindow(800, 600, "Daedalus", NULL, NULL);
    }

    void Engine::initialize_vulkan()
    {
        // Instance and debug messenger creation
        vkb::InstanceBuilder builder;

        auto instance_builder = builder
            .set_debug_callback(vk_debug_callback)
#if defined(DDLS_DEBUG)
            .request_validation_layers()
#endif
            .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            .set_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            .set_engine_name("Daedalus")
            .set_engine_version(DDLS_VERSION_MAJOR, DDLS_VERSION_MINOR, DDLS_VERSION_PATCH)
            .set_app_name(settings.application_name.c_str())
            .set_app_version(0, 1)
            .require_api_version(1, 1, 0)
            .build();

        if (!instance_builder)
            Log::Error("Failed to create Vulkan instance: ", instance_builder.error());

        vkb::Instance vkb_instance = instance_builder.value();

        this->instance = vkb_instance.instance;
        this->debug_messenger = vkb_instance.debug_messenger;

        // Surface creation
        if (glfwCreateWindowSurface(this->instance, this->window, nullptr, &this->surface))
            Log::Error("Failed to create window surface!");

        // Physical device creation
        vkb::PhysicalDeviceSelector physical_device_selector{ vkb_instance };
        auto physical_device = physical_device_selector
            .set_minimum_version(1, 1)
            .set_surface(this->surface)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
#if defined(DDLS_PLATFORM_MACOS)
            .add_required_extension("VK_KHR_portability_subset")
#endif
            .select();

        if (!physical_device)
            Log::Error("Failed to select Vulkan physical device: ", physical_device.error().message());

        // Device creation
        vkb::DeviceBuilder device_builder{ physical_device.value() };
        vkb::Device vkb_device = device_builder.build().value();

        this->device = vkb_device.device;
        this->physical_device = physical_device.value().physical_device;

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(this->physical_device, &device_properties);
        Log::Info(fmt::format("GPU {} properties:", device_properties.deviceName),
            Log::indent, fmt::format("GPU driver version {}.{}.{}", VK_VERSION_MAJOR(device_properties.driverVersion), VK_VERSION_MINOR(device_properties.driverVersion), VK_VERSION_PATCH(device_properties.driverVersion)),
            Log::indent, fmt::format("GPU Vulkan API version {}.{}.{}", VK_VERSION_MAJOR(device_properties.apiVersion), VK_VERSION_MINOR(device_properties.apiVersion), VK_VERSION_PATCH(device_properties.apiVersion)),
            Log::indent, fmt::format("Minimum buffer alignment {}", device_properties.limits.minUniformBufferOffsetAlignment)
        );

        graphics_queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
        graphics_queue_family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();

        VmaAllocatorCreateInfo vma_allocator_info{};
        vma_allocator_info.physicalDevice = this->physical_device;
        vma_allocator_info.device = this->device;
        vma_allocator_info.instance = this->instance;
        vmaCreateAllocator(&vma_allocator_info, &this->allocator);

        main_deletion_queue.push_function([&]() {
            vmaDestroyAllocator(allocator);
        });

        load_meshes();
    }

    void Engine::initialize_swapchain()
    {
        vkb::SwapchainBuilder swapchain_builder{ physical_device, device, surface };

        vkb::Swapchain vkb_swapchain = swapchain_builder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .build()
            .value();

        swapchain = vkb_swapchain.swapchain;
        swapchain_images = vkb_swapchain.get_images().value();
        swapchain_image_views = vkb_swapchain.get_image_views().value();
        settings.window_extent = vkb_swapchain.extent;
        swapchain_image_format = vkb_swapchain.image_format;

        // Depth image size will match the window
        VkExtent3D depth_image_extent = {
            settings.window_extent.width,
            settings.window_extent.height,
            1 };

        // Hardcoding the depth format to 32 bit float
        depth_format = VK_FORMAT_D32_SFLOAT;

        // The depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo depth_image_info = init::image_create_info(depth_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depth_image_extent);

        // For the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo depth_image_allocation_info = {};
        depth_image_allocation_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        depth_image_allocation_info.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Allocate and create the image
        vmaCreateImage(allocator, &depth_image_info, &depth_image_allocation_info, &depth_image.image, &depth_image.allocation, nullptr);

        // Build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo depth_view_info = init::image_view_create_info(depth_format, depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);
        VK_CHECK(vkCreateImageView(device, &depth_view_info, nullptr, &depth_image_view));
    }

    void Engine::initialize_commands()
    {
        VkCommandPoolCreateInfo command_pool_info = init::command_pool_create_info(graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
        VK_CHECK(vkCreateCommandPool(device, &command_pool_info, nullptr, &command_pool));

        VkCommandBufferAllocateInfo command_buffer_allocate_info = init::command_buffer_allocate_info(command_pool, 1);
        VK_CHECK(vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &main_command_buffer));
    }

    void Engine::initialize_default_renderpass()
    {
        // The renderpass will use this color attachment.
        VkAttachmentDescription color_attachment = {};
        // The attachment will have the format needed by the swapchain
        color_attachment.format = swapchain_image_format;
        // 1 sample, we won't be doing MSAA
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // We clear when this attachment is loaded
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // We keep the attachment stored when the renderpass ends
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        // We don't care about stencil
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        // We don't know or care about the starting layout of the attachment
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        // After the renderpass ends, the image has to be on a layout ready for display
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_reference = {};
        // Attachment number will index into the pAttachments array in the parent renderpass itself
        color_attachment_reference.attachment = 0;
        color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_attachment = {};
        depth_attachment.flags = 0;
        depth_attachment.format = depth_format;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachement_reference = {};
        depth_attachement_reference.attachment = 1;
        depth_attachement_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // We are going to create 1 subpass, which is the minimum you can do
        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_reference;
        // Hook the depth attachment into the subpass
        subpass.pDepthStencilAttachment = &depth_attachement_reference;

        // Array of 2 attachments, one for the color, and other for depth
        VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        // Connect the color attachment to the info
        render_pass_info.attachmentCount = 2;
        render_pass_info.pAttachments = &attachments[0];
        // Connect the subpass to the info
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;

        VK_CHECK(vkCreateRenderPass(device, &render_pass_info, nullptr, &render_pass));
    }

    void Engine::initialize_framebuffers()
    {
        // Create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.pNext = nullptr;

        framebuffer_info.renderPass = render_pass;
        framebuffer_info.width = settings.window_extent.width;
        framebuffer_info.height = settings.window_extent.height;
        framebuffer_info.layers = 1;

        // Grab how many images we have in the swapchain
        const uint32_t swapchain_image_count = swapchain_images.size();
        framebuffers = std::vector<VkFramebuffer>(swapchain_image_count);

        // Create framebuffers for each of the swapchain image views
        for (int i = 0; i < swapchain_image_count; i++)
        {
            VkImageView attachments[2];
            attachments[0] = swapchain_image_views[i];
            attachments[1] = depth_image_view;

            framebuffer_info.pAttachments = attachments;
            framebuffer_info.attachmentCount = 2;

            VK_CHECK(vkCreateFramebuffer(device, &framebuffer_info, nullptr, &framebuffers[i]));
        }
    }

    void Engine::initialize_sync_structures()
    {
        // Create syncronization structures
        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.pNext = nullptr;

        // We want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VK_CHECK(vkCreateFence(device, &fence_info, nullptr, &render_fence));

        main_deletion_queue.push_function([=]() {
            vkDestroyFence(device, render_fence, nullptr);
        });

        // For the semaphores we don't need any flags
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_info.pNext = nullptr;
        semaphore_info.flags = 0;

        VK_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &present_semaphore));
        VK_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &render_semaphore));

        // Enqueue the destruction of semaphores
        main_deletion_queue.push_function([=]() {
            vkDestroySemaphore(device, present_semaphore, nullptr);
            vkDestroySemaphore(device, render_semaphore, nullptr);
		});
    }

    void Engine::initialize_pipelines()
    {
        if (!load_shader_module(File::Shader("ColoredTriangle.frag.spv").c_str(), &colored_triangle_frag_shader))
            Log::Error("Error when loading the colored triangle fragment shader module");
        else
            Log::Info("Colored triangle fragment shader succesfully loaded");

        if (!load_shader_module(File::Shader("ColoredTriangle.vert.spv").c_str(), &colored_triangle_vertex_shader))
            Log::Error("Error when loading the colored triangle fragment shader module");
        else
            Log::Info("Colored triangle fragment shader succesfully loaded");

        if (!load_shader_module(File::Shader("Triangle.frag.spv").c_str(), &triangle_frag_shader))
            Log::Error("Error when loading the triangle fragment shader module");
        else
            Log::Info("Triangle fragment shader succesfully loaded");

        if (!load_shader_module(File::Shader("Triangle.vert.spv").c_str(), &triangle_vertex_shader))
            Log::Error("Error when loading the triangle fragment shader module");
        else
            Log::Info("Triangle fragment shader succesfully loaded");

        VkPipelineLayoutCreateInfo pipeline_layout_info = init::pipeline_layout_create_info();

        VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));

        // Build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
        PipelineBuilder pipeline_builder;

        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangle_vertex_shader));

        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangle_frag_shader));

        // Vertex input controls how to read vertices from vertex buffers. We aren't using it yet
        pipeline_builder.vertex_input_info = init::vertex_input_state_create_info();

        // Input assembly is the configuration for drawing triangle lists, strips, or individual points.
        // We are just going to draw triangle list
        pipeline_builder.input_assembly = init::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        // Build viewport and scissor from the swapchain extents
        pipeline_builder.viewport.x = 0.0f;
        pipeline_builder.viewport.y = 0.0f;
        pipeline_builder.viewport.width = (float)settings.window_extent.width;
        pipeline_builder.viewport.height = (float)settings.window_extent.height;
        pipeline_builder.viewport.minDepth = 0.0f;
        pipeline_builder.viewport.maxDepth = 1.0f;

        pipeline_builder.scissor.offset = { 0, 0 };
        pipeline_builder.scissor.extent = settings.window_extent;

        // Configure the rasterizer to draw filled triangles
        pipeline_builder.rasterizer = init::rasterization_state_create_info(VK_POLYGON_MODE_FILL);

        // We don't use multisampling, so just run the default one
        pipeline_builder.multisampling = init::multisampling_state_create_info();

        // A single blend attachment with no blending and writing to RGBA
        pipeline_builder.color_blend_attachment = init::color_blend_attachment_state();

        // Use the triangle layout we created
        pipeline_builder.pipeline_layout = pipeline_layout;

        pipeline_builder.depth_stencil = init::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

        // Finally build the pipeline
        triangle_pipeline = pipeline_builder.buildPipeline(device, render_pass);

        pipeline_builder.shader_stages.clear();

        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_VERTEX_BIT, colored_triangle_vertex_shader));

        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, colored_triangle_frag_shader));

        colored_triangle_pipeline = pipeline_builder.buildPipeline(device, render_pass);

        // Build the mesh pipeline
        VertexInputDescription vertex_description = Vertex::get_vertex_description();

        // Connect the pipeline builder vertex input info to the one we get from Vertex
        pipeline_builder.vertex_input_info.pVertexAttributeDescriptions = vertex_description.attributes.data();
        pipeline_builder.vertex_input_info.vertexAttributeDescriptionCount = vertex_description.attributes.size();

        pipeline_builder.vertex_input_info.pVertexBindingDescriptions = vertex_description.bindings.data();
        pipeline_builder.vertex_input_info.vertexBindingDescriptionCount = vertex_description.bindings.size();

        // Clear the shader stages for the builder
        pipeline_builder.shader_stages.clear();

        // Compile mesh vertex shader
        if (!load_shader_module(File::Shader("TriangleMesh.vert.spv").c_str(), &mesh_vertex_shader))
            Log::Error("Error when loading the triangle mesh vertex shader module");
        else
            Log::Info("Triangle mesh vertex shader succesfully loaded");

        // Add the other shaders
        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_VERTEX_BIT, mesh_vertex_shader));

        // Make sure that triangleFragShader is holding the compiled colored_triangle.frag
        pipeline_builder.shader_stages.push_back(
            init::pipeline_shader_state_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, colored_triangle_frag_shader));

        // We start from just the default empty pipeline layout info
        pipeline_layout_info = init::pipeline_layout_create_info();

        // Setup push constants
        VkPushConstantRange push_constant;
        // This push constant range starts at the beginning
        push_constant.offset = 0;
        // This push constant range takes up the size of a MeshPushConstants struct
        push_constant.size = sizeof(MeshPushConstants);
        // This push constant range is accessible only in the vertex shader
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pipeline_layout_info.pPushConstantRanges = &push_constant;
        pipeline_layout_info.pushConstantRangeCount = 1;

        VK_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &mesh_pipeline_layout));

        pipeline_builder.pipeline_layout = mesh_pipeline_layout;

        // Build the mesh pipeline
        mesh_pipeline = pipeline_builder.buildPipeline(device, render_pass);

        create_material(mesh_pipeline, mesh_pipeline_layout, "defaultmesh");

        // Deleting all of the vulkan shaders
        vkDestroyShaderModule(device, mesh_vertex_shader, nullptr);
        vkDestroyShaderModule(device, triangle_vertex_shader, nullptr);
        vkDestroyShaderModule(device, triangle_frag_shader, nullptr);
        vkDestroyShaderModule(device, colored_triangle_frag_shader, nullptr);
        vkDestroyShaderModule(device, colored_triangle_vertex_shader, nullptr);
    }

    bool Engine::load_shader_module(const char* file_path, VkShaderModule* shader_module)
    {
        std::ifstream file(file_path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            return false;
        }

        // Find what the size of the file is by looking up the location of the cursor
        // Because the cursor is at the end, it gives the size directly in bytes
        size_t fileSize = (size_t)file.tellg();

        // SPIR-V expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        // Put file cursor at beggining
        file.seekg(0);

        // Load the entire file into the buffer
        file.read((char*)buffer.data(), fileSize);

        // Now that the file is loaded into the buffer, we can close it
        file.close();

        // Create a new shader module, using the buffer we loaded
        VkShaderModuleCreateInfo shader_module_info = {};
        shader_module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_info.pNext = nullptr;

        // codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
        shader_module_info.codeSize = buffer.size() * sizeof(uint32_t);
        shader_module_info.pCode = buffer.data();

        // Check that the creation goes well.
        VkShaderModule result;
        if (vkCreateShaderModule(device, &shader_module_info, nullptr, &result) != VK_SUCCESS)
            return false;
        *shader_module = result;

        return true;
    }

    void Engine::load_meshes()
    {
        // Make the array 3 vertices long
        triangle_mesh.vertices.resize(3);

        // Vertex positions
        triangle_mesh.vertices[0].position = { .5f, .5f, 0.0f };
        triangle_mesh.vertices[1].position = { -.5f, .5f, 0.0f };
        triangle_mesh.vertices[2].position = { 0.f, -1.f, 0.0f };

        triangle_mesh.vertices[0].color = { 0.f, 0.f, 1.f };
        triangle_mesh.vertices[1].color = { 0.f, 0.f, 1.f };
        triangle_mesh.vertices[2].color = { 0.75f, 0.75f, 1.f };

        monkey_mesh.load_from_obj(File::Asset("MonkeyFlat.obj").c_str());

        upload_mesh(triangle_mesh);
        upload_mesh(monkey_mesh);

        this->meshes["triangle"] = triangle_mesh;
        this->meshes["monkey"] = monkey_mesh;
    }

    void Engine::upload_mesh(Mesh& mesh)
    {
        // Allocate vertex buffer
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // This is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = mesh.vertices.size() * sizeof(Vertex);
        // This buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        // Let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo vma_allocation_info = {};
        vma_allocation_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        // Allocate the buffer
        VK_CHECK(vmaCreateBuffer(allocator, &buffer_info, &vma_allocation_info,
            &mesh.vertex_buffer.buffer,
            &mesh.vertex_buffer.allocation,
            nullptr));

        // Add the destruction of triangle mesh buffer to the deletion queue
        main_deletion_queue.push_function([=]()
            { vmaDestroyBuffer(allocator, mesh.vertex_buffer.buffer, mesh.vertex_buffer.allocation); });

        // Copy vertex data
        void* data;
        vmaMapMemory(allocator, mesh.vertex_buffer.allocation, &data);

        memcpy(data, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

        vmaUnmapMemory(allocator, mesh.vertex_buffer.allocation);
    }

    Material* Engine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name)
    {
        Material material;
        material.pipeline = pipeline;
        material.pipeline_layout = layout;
        this->materials[name] = material;
        return &this->materials[name];
    }

    Material* Engine::get_material(const std::string& name)
    {
        // Search for the object, and return nullptr if not found
        auto it = this->materials.find(name);
        if (it == this->materials.end())
            return nullptr;
        else
            return &(*it).second;
    }

    Mesh* Engine::get_mesh(const std::string& name)
    {
        auto it = this->meshes.find(name);
        if (it == this->meshes.end())
            return nullptr;
        else
            return &(*it).second;
    }

    void Engine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count)
    {
        // Make a model view matrix for rendering the object
        // Camera view
        glm::vec3 position = { 0.f, -6.f, -10.f };

        glm::mat4 view = glm::translate(glm::mat4(1.f), position);
        // Camera projection
        glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
        projection[1][1] *= -1;
        
        Mesh* last_mesh = nullptr;
        Material* last_material = nullptr;
        for (int i = 0; i < count; i++)
        {
            RenderObject& object = first[i];

            // Only bind the pipeline if it doesn't match with the already bound one
            if (object.material != last_material)
            {

                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
                last_material = object.material;
            }

            glm::mat4 model = object.transform_matrix;
            // Final render matrix, that we are calculating on the cpu
            glm::mat4 mesh_matrix = projection * view * model;

            MeshPushConstants constants;
            constants.render_matrix = mesh_matrix;

            // Upload the mesh to the GPU via push constants
            vkCmdPushConstants(cmd, object.material->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

            // Only bind the mesh if it's a different one from last bind
            if (object.mesh != last_mesh)
            {
                // Bind the mesh vertex buffer with offset 0
                VkDeviceSize offset = 0;
                vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->vertex_buffer.buffer, &offset);
                last_mesh = object.mesh;
            }
            // We can now draw
            vkCmdDraw(cmd, object.mesh->vertices.size(), 1, 0, 0);
        }
    }

    void Engine::init_scene()
    {
        RenderObject monkey;
        monkey.mesh = get_mesh("monkey");
        monkey.material = get_material("defaultmesh");
        monkey.transform_matrix = glm::mat4{ 1.0f };

        this->renderables.push_back(monkey);

        for (int x = -20; x <= 20; x++)
        {
            for (int y = -20; y <= 20; y++)
            {
                RenderObject tri;
                tri.mesh = get_mesh("triangle");
                tri.material = get_material("defaultmesh");
                glm::mat4 translation = glm::translate(glm::mat4{ 1.0 }, glm::vec3(x, 0, y));
                glm::mat4 scale = glm::scale(glm::mat4{ 1.0 }, glm::vec3(0.2, 0.2, 0.2));
                tri.transform_matrix = translation * scale;

                this->renderables.push_back(tri);
            }
        }
    }
}