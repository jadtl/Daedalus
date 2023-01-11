#pragma once

#if DDLS_DEBUG

// Define some architecture-specific inline assembly that causes a break
#if __x86_64__

#define debugBreak() { __asm__ __volatile__("int3"); }

#elif __aarch64__

#define debugBreak() { __asm__ __volatile__(".inst 0xd4200000"); }

// TODO: Support more architectures
#endif

#else

// Evaluates to nothing
#define debugBreak() {}

#endif