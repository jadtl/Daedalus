#pragma once

#include <vulkan/vulkan.hpp>

#include "ValidationLayers.hpp"

namespace ddls
{
void createInstance(VkInstance& instance,
                    const std::vector<const char*> validationLayers,
                    const std::string applicationName,
                    const std::string engineName);
}
