#include <daedalus.h>

#include <limits>
#include <iostream>

using namespace ddls;

int main()
{

    u32 n1 = 0x12'34'56'78;
    n1 = Endian::swapU32(n1);
    ASSERT(n1 == 0x78'56'34'12)

    union _1 { u32 ui; i32 si; };
    _1 u1;
    u1.ui = Endian::swapU32(0b1111'1111'1111'1111'1111'1111'1111'0000);
    u1.si = Endian::swapI32(u1.si);
    ASSERT(u1.si == -16)

    union _2 { u32 i; f32 f; };
    _2 u2;
    u2.i = Endian::swapU32(0x12'34'56'78);
    ASSERT(abs(u2.f - Endian::swapF32((f32)5.6904566139e-28))
        < std::numeric_limits<f32>::epsilon())

    union _3 { u64 i; f64 f; };
    _3 u3;
    u3.i = Endian::swapU64(0x12'34'56'78);
    ASSERT(abs(u3.f - Endian::swapF64((f64)1.50897478170006352032767046355E-315))
        < std::numeric_limits<f64>::epsilon())

    std::cout << "Test passed!" << '\n';
    return Success;
}