#pragma once

#include <vulkan/vulkan.hpp>

#include "ValidationLayers.hpp"
#include "PhysicalDevice.hpp"
#include "QueueFamily.hpp"

#include <set>

namespace ddls
{
void createLogicalDevice(VkDevice& device,
                         VkPhysicalDevice physicalDevice,
                         VkQueue& graphicsQueue,
                         VkQueue& presentQueue,
                         VkSurfaceKHR& surface,
                         const std::vector<const char*>& validationLayers);
}
