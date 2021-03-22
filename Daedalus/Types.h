#pragma once

#include <vulkan/vulkan.h>

#include "MemoryAllocator.h"

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};
