#pragma once

#include <vulkan/vulkan.h>

#include "MemoryAllocator.h"

struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage {
    VkImage image;
    VmaAllocation allocation;
};
