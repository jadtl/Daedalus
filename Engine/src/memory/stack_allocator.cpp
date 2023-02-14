#include "memory/stack_allocator.h"

#include <cstdlib>
#include <cstring>

namespace ddls {

StackAllocator::StackAllocator(MemSize size)
{
    _spInit = (Ptr) malloc(size);
    _sp = _spInit;
    _spMin = _sp;
    _spMax = _sp + size;
    if (!_sp) throw OutOfMemoryException("Failed to allocate stack!");
}

StackAllocator::~StackAllocator()
{
    free(_spInit);
}

Ptr StackAllocator::allocate(MemSize size, Boolean clear)
{
    if (_sp + size > _spMax) throw OutOfMemoryException("Stack is full!");
    if (clear) memset(_sp, 0, size);
    _sp += size;
    return _sp - size;
}

void StackAllocator::free(Ptr ptr)
{
    if (!(_spMin <= ptr && ptr <= _sp)) throw OutOfBoundsException("The requested free location isn't on the stack!");
    _sp = ptr;
}

} // namespace ddls
