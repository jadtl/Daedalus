#pragma once

#include <iostream>

#if DDLS_ASSERT

#define reportAssertionFailure(expr, file, line) \
    std::cout << "Assertion \"" << expr << "\" failed in " \
        << file << " at line " << line << "\n";

// Check the expression and fail if it is false
#define ASSERT(expr) \
    if (expr) {} \
    else \
    { \
        reportAssertionFailure(#expr, __FILE__, __LINE__); \
        debugBreak(); \
    }

#else

// Evaluates to nothing
#define ASSERT(expr) 

#endif