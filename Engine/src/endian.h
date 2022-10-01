#pragma once

#include "types.h"

namespace ddls {

inline u16 swapU16(u16 value)
{
    return (u16)((value << 8) | (value >> 8));
}

inline u32 swapU32(u32 value)
{
    return ((value & 0x00'00'00'FF) << 24) 
        | ((value & 0x00'00'FF'00) << 8)
        | ((value & 0x00'FF'00'00) >> 8) 
        | ((value & 0xFF'00'00'00) >> 24);
}

inline u64 swapU64(u64 value)
{
    return ((value & 0x00'00'00'00'00'00'00'FF) << 56)
        | ((value & 0x00'00'00'00'00'00'FF'00) << 40)
        | ((value & 0x00'00'00'00'00'FF'00'00) << 24)
        | ((value & 0x00'00'00'00'FF'00'00'00) << 8)
        | ((value & 0x00'00'00'FF'00'00'00'00) >> 8)
        | ((value & 0x00'00'FF'00'00'00'00'00) >> 24)
        | ((value & 0x00'FF'00'00'00'00'00'00) >> 40)
        | ((value & 0xFF'00'00'00'00'00'00'00) >> 56);
}

inline i16 swapI16(i16 value)
{
    return (i16)((value << 8) | ((value >> 8) & 0x00'FF));
}

inline i32 swapI32(i32 value)
{
    return ((value & 0x00'00'00'FF) << 24) 
        | ((value & 0x00'00'FF'00) << 8)
        | ((value >> 8) & 0x00'00'FF'00) 
        | ((value >> 24) & 0x00'00'00'FF);
}

inline i64 swapI64(i64 value)
{
    return ((value & 0x00'00'00'00'00'00'00'FF) << 56)
        | ((value & 0x00'00'00'00'00'00'FF'00) << 40)
        | ((value & 0x00'00'00'00'00'FF'00'00) << 24)
        | ((value & 0x00'00'00'00'FF'00'00'00) << 8)
        | ((value >> 8) & 0x00'00'00'00'FF'00'00'00)
        | ((value >> 24) & 0x00'00'00'00'00'FF'00'00)
        | ((value >> 40) & 0x00'00'00'00'00'00'FF'00) 
        | ((value >> 56) & 0x00'00'00'00'00'00'00'FF);
}

inline f32 swapF32(f32 value)
{
    /**
     * A union that helps reinterpreting types while avoiding punning bugs
     * https://www.cocoawithlove.com/2008/04/using-pointers-to-recast-in-c-is-bad.html
     */
    union _ { u32 i; f32 f; };

    _ u;
    u.f = value;
    u.i = swapU32(u.i);

    return u.f;
}

inline f64 swapF64(f64 value)
{
    /**
     * A union that helps reinterpreting types while avoiding punning bugs
     * https://www.cocoawithlove.com/2008/04/using-pointers-to-recast-in-c-is-bad.html
     */
    union _ { u64 i; f64 f; };

    _ u;
    u.f = value;
    u.i = swapU64(u.i);

    return u.f;
}

} // namespace ddls