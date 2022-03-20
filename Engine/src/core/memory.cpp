#include <core/memory.h>

#include <cstring>
#include <memory>

namespace ddls {
void* zero_memory(void* block, u64 size) {
    return memset(block, 0, size);
}
}