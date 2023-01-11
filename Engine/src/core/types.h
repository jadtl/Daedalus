#pragma once

#include <stdint.h>

namespace ddls {

/** @brief A unsigned 8-bit integer */
using u8 = uint8_t;

/** @brief A unsigned 16-bit integer */
using u16 = uint16_t;

/** @brief A unsigned 32-bit integer */
using u32 = uint32_t;

/** @brief A unsigned 64-bit integer */
using u64 = uint64_t;


/** @brief A signed 8-bit integer */
using i8 = int8_t;

/** @brief A signed 16-bit integer */
using i16 = int16_t;

/** @brief A signed 32-bit integer */
using i32 = int32_t;

/** @brief A signed 64-bit integer */
using i64 = int64_t;

/** @brief A unsigned integer at least 32-bit */
using u32_fast = uint_fast32_t;

/** @brief A signed integer at least 32-bit */
using i32_fast = int_fast32_t;


/** @brief An IEEE-754 32-bit floating-point number */
using f32 = float;

/** @brief An IEEE-754 64-bit floating-point number */
using f64 = double;


/** @brief A boolean type */
using b8 = bool;


/** @brief A pointer type */
using ptr = uintptr_t;

} // namespace ddls