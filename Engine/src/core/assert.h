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

#define Assert(expr, args)\
    if (expr) {}\
    else {\
        Log::Out(Log::Styles::Default, Log::Colours::Magenta, Log::Level::Assert, args);\
        reportAssertionFailure(#expr, __FILE__, __LINE__);\
        debugBreak();\
    }\

#else

// Evaluates to nothing
#define ASSERT(expr)

#define Assert(expr, args)

#endif
}