#include <CommandPool.hpp>

namespace ddls
{
void createCommandPool(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkCommandPool& commandPool)
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    // Because command buffers can be very flexible, we don't want to be 
    // doing a lot of allocation while we're trying to render.
    // For this reason we create a pool to hold allocated command buffers.
    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // This allows the command buffer to be implicitly reset when vkBeginCommandBuffer is called.
    // You can also explicitly call vkResetCommandBuffer.  
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // We'll be building command buffers to send to the graphics queue
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    
    if (vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}
}
