#include <daedalus/memory.h>

#include <memory>

namespace ddls {
    void* zero_memory(void* block, u64 size) {
        return memset(block, 0, size);
    }
}