#pragma once

#include "types.h"

namespace ddls
{
/**
 * @brief Type for a memory size in bytes
 * 
 */
using MemSize = u32;
static MemSize MemSize_default = 1024;

/**
 * @brief Type for pointers
 * 
 */
using Ptr = u32*;

/**
 * @brief Memory tags to help profile subsystems memory consumption
 * 
 */
enum MemoryTag
{
    Logging,
    Miscellaneous
};
}