#include <daedalus.h>
#include <memory/stack_allocator.h>

#include "test.h"

using namespace ddls;

int main()
{
    StackAllocator allocator(16*sizeof(u32));
    Ptr arr = allocator.allocate(8*sizeof(f32));
    arr[1] = 4.0f;
    allocator.free(arr);
    f32 tmp = (f32)(arr+sizeof(f32))[0];
    ASSERT(!((arr+sizeof(f32))[0]))

    arr = allocator.allocate(2*sizeof(f32), false);
    ASSERT(!((f32)arr[1] != 4.0f))

    ASSERT_THROWS(allocator.free(0), OutOfBoundsException)

    TEST_SUCCESS
}
