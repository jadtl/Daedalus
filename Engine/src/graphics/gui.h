#pragma once

#include "core/defines.h"
#include "graphics/pipeline.h"
#include "graphics/swapchain.h"
#include "core/types.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace ddls
{
class DDLS_API Gui
{
public:
    Gui(
        GLFWwindow *window,
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkQueue queue,
        u32 queueFamilyIndex,
        ddls::Swapchain *swapchain,
        b8 docking = false);

    /**
     * @brief Destroys the GUI's context
     * 
     */
    ~Gui();

    /**
     * @brief Acquires a new frame to render
     * 
     */
    void newFrame();

    /**
     * @brief Renders the next frame and updates windows
     * 
     */
    void render();

    std::vector<VkCommandBuffer> commandBuffers() const { return _commandBuffers; }
    void recordCommands(u32 currentFrame, u32 imageIndex, Swapchain *swapchain);

private:
    VkDevice _device;
    VkDescriptorPool _descriptorPool;
    ddls::Swapchain *_swapchain;
    VkQueue _queue;
    u32 _queueFamilyIndex;
    std::vector<VkFramebuffer> _framebuffers;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    VkRenderPass _renderPass;
    ImGuiIO _io;
    b8 _docking;

    void createDescriptorPool();
    void createCommands();
    void createRenderPass();

    void createFramebuffers();
    void destroyFramebuffers();

    void uploadFonts();
};
}