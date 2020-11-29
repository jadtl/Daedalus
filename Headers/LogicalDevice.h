#pragma once

#include <vulkan/vulkan.hpp>

#include "ValidationLayers.h"
#include "PhysicalDevice.h"
#include "QueueFamily.h"

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
