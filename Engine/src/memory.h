#pragma once

#include "types.h"

namespace ddls
{
using MemSize = u32;
using Ptr = u32*;

enum MemoryTag
{
    Logging,
    Miscellaneous
};
}