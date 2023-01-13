#pragma once

#include "core/assert.h"
#include "core/debug.h"

#include <iostream>

namespace ddls {
#if DDLS_ASSERT

#define reportAssertionFailure(expr, file, line) \
    ddls::Log::Error("Assertion \"", expr, "\" failed in ", file, ":", line);

// Check the expression and fail if it is false
#define ASSERT(expr) \
    if (expr) {} \
    else \
    { \
        reportAssertionFailure(#expr, __FILE__, __LINE__); \
        debugBreak(); \
    }

#define ASSERT_TRACE(expr, file, line) \
    if (expr) {} \
    else \
    { \
        reportAssertionFailure(#expr, file, line); \
        debugBreak(); \
    }

#else

// Evaluates to nothing
#define ASSERT(expr)

#define ASSERT_TRACE(expr)

#endif
}