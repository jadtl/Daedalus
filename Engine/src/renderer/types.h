#pragma once

#include <memory/allocator.h>

struct AllocatedBuffer
{
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage
{
    VkImage image;
    VmaAllocation allocation;
};
