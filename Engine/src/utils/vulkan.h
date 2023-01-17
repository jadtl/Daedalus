#pragma once

#include "core/defines.h"
#include "core/log.h"
#include "utils/helper.h"

#include <vulkan/vulkan.h>

#include <string>
#include <vector>
#include <optional>

namespace ddls
{
class DDLS_API Vulkan: public Helper
{
public:
    static VkShaderModule loadShader(VkDevice device,
        std::string path)
    {
        std::vector<char> code = readFile(path);
        VkShaderModuleCreateInfo shaderCreateInfo{};
        shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderCreateInfo.codeSize = (u32)code.size();
        shaderCreateInfo.pCode = reinterpret_cast<const u32*>(code.data());

        VkShaderModule shaderModule;
        Assert(vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &shaderModule) == VK_SUCCESS,
            "Failed to create shader module!");

        return shaderModule;
    }
    static std::vector<char> readFile(
        const std::string& fileName)
    {
        // Start reading at the end of the file and avoid text transformations
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        Assert(file.is_open(),
            fmt::format("Failed to open file {}!", fileName));

        u32 fileSize = (u32)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;
        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    static QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        // Finding a queue that supports graphics commands
        QueueFamilyIndices indices;
        u32 i = 0;
        for (const auto& queueFamily: queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) break;

            i++;
        }

        return indices;
    }

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    static SwapchainSupportDetails querySwapchainSupport(
        VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

        u32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }

        u32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    static void createCommandPool(
        VkDevice device,
        VkCommandPool *commandPool,
        u32 queueFamilyIndex,
        VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        commandPoolCreateInfo.flags = flags;

        Assert(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, commandPool) == VK_SUCCESS,
            "Failed to create ImGui command pool!");
    }

    static void createCommandBuffers(
        VkDevice device,
        VkCommandBuffer *commandBuffer, 
        u32 commandBufferCount, 
        VkCommandPool& commandPool)
    {
        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = commandBufferCount;

        vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffer);
    }

    static void createFramebuffers(
        VkDevice device,
        VkFramebuffer *framebuffer,
        u32 framebufferCount,
        VkRenderPass renderPass,
        VkExtent2D extent,
        VkImageView *imageView)
    {
        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = renderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = extent.width;
        info.height = extent.height;
        info.layers = 1;

        for (u32 i = 0; i < framebufferCount; i++)
        {
            attachment[0] = imageView[i];

            Assert(vkCreateFramebuffer(device, &info, nullptr, &framebuffer[i]) == VK_SUCCESS,
                "Failed to create ImGui framebuffer!");
        }
    }
};
}