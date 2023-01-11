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
     * @brief Allocates memory to use as a stack
     * 
     * 
     * @param size The size of the memory region
     */
    explicit StackAllocator(MemSize size = MemSize_default);

    /**
     * @brief Frees the allocated memory region
     * 
     */
    ~StackAllocator();

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
    void free(Ptr ptr);

private:
    Ptr _spInit;
    Ptr _sp;
    Ptr _spMin;
    Ptr _spMax;
};
}
