#pragma once

#include "defines.h"

// Disable assertions by commenting out this line
#define DDLS_ASSERTIONS_ENABLED

#ifdef DDLS_ASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

DDLS_API void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define DDLS_ASSERT(expr)                                      \
  {                                                            \
    if (expr) {                                                \
    } else {                                                   \
      report_assertion_failure(#expr, "", __FILE__, __LINE__); \
      debugBreak();                                            \
    }                                                          \
  }                                                            \

#define DDLS_ASSERT_MSG(expr, message)                              \
  {                                                                 \
    if (expr) {                                                     \
    } else {                                                        \
      report_assertion_failure(#expr, message, __FILE__, __LINE__); \
      debugBreak();                                                 \
    }                                                               \
  }                                                                 \

#ifdef _DEBUG
#define DDLS_ASSERT_DEBUG(expr)                                \
  {                                                            \
    if (expr) {                                                \
    } else {                                                   \
      report_assertion_failure(#expr, "", __FILE__, __LINE__); \
      debugBreak();                                            \
    }                                                          \
  }                                                            \

#else
#define DDLS_ASSERT_DEBUG(expr)
#endif

#else
#define DDLS_ASSERT(expr)
#define DDLS_ASSERT_MSG(expr, message)
#define DDLS_ASSERT_DEBUG(expr)
#endif