#ifndef MEM_ALLOCATOR_HPP
#define MEM_ALLOCATOR_HPP
#include "../lib/hw.h"

struct BlockHeader {
    BlockHeader* next;
    size_t size;
};


class MemAllocator {
public:
    static MemAllocator& getInstance();
    void* malloc(size_t size);
    void mem_free(void* ptr);
    size_t findBiggestFreeBlock();
    size_t sizeOfFreeSpace();
    MemAllocator();
    MemAllocator(const MemAllocator& other) = delete;
    MemAllocator& operator= (const MemAllocator& other) = delete;

private:

    //static BlockHeader* ffreeMemHead;

    BlockHeader* freeMemHead;

    void join(BlockHeader* blk);
};

#endif