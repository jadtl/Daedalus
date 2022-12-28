#include <daedalus.h>
#include <memory/stack_allocator.h>

using namespace ddls;

int main()
{
    StackAllocator allocator;
    allocator.initialize(16*sizeof(u32));
    Ptr arr = allocator.allocate(8*sizeof(f32));
    arr[1] = 4.0f;
    allocator.free(arr);
    f32 tmp = (arr+sizeof(f32))[0];
    if ((arr+sizeof(f32))[0]) return Failure;
    arr = allocator.allocate(2*sizeof(f32), false);
    if (arr[1] != 4.0f) return Failure;
    std::cout << "Test passed!" << '\n';
    return Success;
}