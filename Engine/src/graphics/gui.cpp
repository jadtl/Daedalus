#include "graphics/gui.h"

#include "core/log.h"
#include "utils/helper.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include "gui.h"

namespace ddls
{
Gui::Gui(
    GLFWwindow *window,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkQueue queue,
    u32 queueFamilyIndex,
    ddls::Swapchain *swapchain,
    b8 docking) : _device(device), _swapchain(swapchain), _queue(queue), _queueFamilyIndex(queueFamilyIndex)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsClassic();

#if defined(IMGUI_HAS_VIEWPORT) && defined(IMGUI_HAS_DOCK)
    if (docking)
    {
        _io = ImGui::GetIO();
            (void)_io;
            _io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            _io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            _io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGuiStyle& style = ImGui::GetStyle();
        if (_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
    }
#else
    if (docking) Log::Error("Docking was requested but not supported by this version of ImGui!");
#endif

    ImGui_ImplGlfw_InitForVulkan(window, true);

    createDescriptorPool();
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.Queue = queue;
    initInfo.DescriptorPool = _descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = swapchain->imageCount();
    createRenderPass();
    ImGui_ImplVulkan_Init(&initInfo, _renderPass);

    createCommands();
    createFramebuffers();
    _swapchain->addFramebuffersRecreateCallback(_framebuffers.data(), _renderPass);

    uploadFonts();
}

ddls::Gui::~Gui()
{
    destroyFramebuffers();
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    // This also frees the command buffers
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::newFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::render()
{
    ImGui::Render();

#if defined(IMGUI_HAS_VIEWPORT) && defined(IMGUI_HAS_DOCK)
    // Update and Render additional Platform Windows
    if (_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
#endif
}

void Gui::createDescriptorPool()
{
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    pool_info.poolSizeCount = (u32)IM_ARRAYSIZE(poolSizes);
    pool_info.pPoolSizes = poolSizes;

    Assert(vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPool) == VK_SUCCESS,
        "Failed to create ImGui descriptor pool!");
}
void Gui::createCommands()
{
    vk::createCommandPool(_device, &_commandPool, _queueFamilyIndex, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    _commandBuffers.resize(_swapchain->imageCount());
    vk::createCommandBuffers(_device, _commandBuffers.data(), (u32)_commandBuffers.size(), _commandPool);
}
void Gui::createRenderPass()
{
    VkAttachmentDescription attachment = {};
    attachment.format = _swapchain->format();
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    Assert(vkCreateRenderPass(_device, &info, nullptr, &_renderPass) == VK_SUCCESS,
        "Failed to create the GUI's render pass!");
}
void Gui::createFramebuffers()
{
    _framebuffers.resize(_swapchain->imageCount());
    vk::createFramebuffers(
        _device, 
        _framebuffers.data(), 
        _swapchain->imageCount(),
        _renderPass,
        _swapchain->extent(),
        _swapchain->imageViews().data());
}
void Gui::destroyFramebuffers()
{
    for (auto framebuffer: _framebuffers)
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
}

void ddls::Gui::uploadFonts()
{
    VkCommandBuffer commandBuffer = _commandBuffers[0];

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkEndCommandBuffer(commandBuffer);

    Assert(vkQueueSubmit(_queue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
        "Failed to submit to queue!");

    vkDeviceWaitIdle(_device);

    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::recordCommands(u32 currentFrame, u32 imageIndex, Swapchain *swapchain)
{
    vkResetCommandBuffer(_commandBuffers[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    Assert(vkBeginCommandBuffer(_commandBuffers[currentFrame], &beginInfo) == VK_SUCCESS,
        "Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain->extent();

    vkCmdBeginRenderPass(_commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(swapchain->extent().width);
    viewport.height = static_cast<f32>(swapchain->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->extent();
    vkCmdSetScissor(_commandBuffers[currentFrame], 0, 1, &scissor);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers[currentFrame]);

    vkCmdEndRenderPass(_commandBuffers[currentFrame]);

    Assert(vkEndCommandBuffer(_commandBuffers[currentFrame]) == VK_SUCCESS,
        "Failed to record command buffer");
}

}