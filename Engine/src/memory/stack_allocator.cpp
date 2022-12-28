#include "memory/stack_allocator.h"

#include <cstdlib>
#include <cstring>

namespace ddls
{
    RetCode StackAllocator::initialize(MemSize size)
    {
        sp = (Ptr)malloc(size);
        spMin = sp;
        spMax = sp + size;
        return sp ? Success : OutOfMemory;
    }

    Ptr StackAllocator::allocate(MemSize size, Boolean clear)
    {
        if (!isInitialized) return nullptr;
        if (sp + size > spMax) return nullptr;
        if (clear) memset(sp, 0, size);
        sp += size;
        return sp - size;
    }

    RetCode StackAllocator::free(Ptr ptr)
    {
        if (!isInitialized) return Failure;
        if (!(spMin <= ptr && ptr <= sp)) return Failure;
        sp = ptr;
        return Success;
    }
}