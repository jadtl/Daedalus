#pragma once

#include "defines.h"
#include "memory.h"
#include "error.h"

namespace ddls
{
class DDLS_API StackAllocator
{
public:
    /**
     * @brief Reserves the a memory region of the given size for allocations
     * 
     * @param size The size of the memory region
     * @return RetCode An error code
     */
    RetCode initialize(MemSize size);

    /**
     * @brief Allocates memory to a given object
     * 
     * @param size The size of the allocated object
     * @param tag The memory tag of the allocated object
     * @return u32 A pointer to the allocated memory location
     */
    Ptr allocate(MemSize size, Boolean clear = true);

    /**
     * @brief Frees objects by moving the stack pointer
     * 
     * @param ptr The new stack pointer
     * @return u8 An error code, Failure if the new stack pointer is not valid
     */
    RetCode free(Ptr ptr);

private:
    Ptr sp;
    Ptr spMin;
    Ptr spMax;
    Boolean isInitialized;
};
}