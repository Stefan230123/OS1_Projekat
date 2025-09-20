#include "../h/memAllocator.hpp"
#include "../test/printing.hpp"
//BlockHeader* MemAllocator::ffreeMemHead = nullptr;

MemAllocator& MemAllocator::getInstance() {
    static MemAllocator instance;
    return instance;
}

MemAllocator::MemAllocator() {
    freeMemHead = (BlockHeader *) HEAP_START_ADDR;
    freeMemHead->next = nullptr;
    freeMemHead->size = (size_t) (((char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR - 1) - MEM_BLOCK_SIZE);
}

void *MemAllocator::malloc(size_t size) {
    BlockHeader *blk = freeMemHead;
    BlockHeader *prev = nullptr;
    size += sizeof(BlockHeader);
    if (size % MEM_BLOCK_SIZE != 0)
        size = (size / MEM_BLOCK_SIZE + 1) * MEM_BLOCK_SIZE;
    for (; blk != nullptr; prev = blk, blk = blk->next) {
        if (blk->size >= size)
            break;
    }

    if (blk == nullptr)
        return nullptr;
    else {
        if (blk->size - size >= MEM_BLOCK_SIZE + sizeof(BlockHeader)) {
            BlockHeader* reminder = (BlockHeader*) ((char*) blk + size);
            reminder->next = blk->next;
            reminder->size = blk->size - size - sizeof(BlockHeader);
            blk->size = size;
            if (prev)
                prev->next = reminder;
            else
                freeMemHead = reminder;
            return (void*) ((char*) blk + sizeof(BlockHeader));
        }
        else
        {
            if(prev)
                prev->next = blk->next;
            else
                freeMemHead = blk->next;
            return (void*) ((char*) blk + sizeof(BlockHeader));
        }
    }
}

void MemAllocator::mem_free(void *ptr) {
    BlockHeader* blk = (BlockHeader*) ((char*) ptr - sizeof(BlockHeader));
    BlockHeader* prev = freeMemHead;

    if (!freeMemHead || blk < freeMemHead) {
        prev = nullptr;
    }
    else {
        for(; prev->next != nullptr && prev < blk; prev = prev->next);
    }
    if (!prev) {
        blk->next = freeMemHead;
        freeMemHead = blk;
    }
    else {
        blk->next = prev->next;
        prev->next = blk;
    }

    join(prev);
    join(blk);
}

size_t MemAllocator::findBiggestFreeBlock() {
    BlockHeader* blk = freeMemHead;
    size_t max = blk->size;
    for (; blk->next != nullptr; blk = blk->next) {
        if (max < blk->size)
            max = blk->size;
    }
    return max;
}

size_t MemAllocator::sizeOfFreeSpace() {
    BlockHeader* blk = freeMemHead;
    size_t sum = 0;
    for (; blk->next != nullptr; blk = blk->next) {
        sum += blk->size;
    }
    return sum;
}

void MemAllocator::join(BlockHeader* blk) {
    if (!blk)
        return;
    if (blk->next && (char*)blk + blk->size + sizeof(BlockHeader) == (char*) blk->next) {
        blk->size += blk->next->size;
        blk->next = blk->next->next;
    }
}