#pragma once

#include "types.h"

namespace ddls
{
enum RetCode
{
    Success,
    Failure,
    OutOfMemory,
};

using Boolean = u8;
}