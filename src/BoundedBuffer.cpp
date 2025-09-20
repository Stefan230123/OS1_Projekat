#include "../h/BoundedBuffer.hpp"

BoundedBuffer::BoundedBuffer(int size) {
    this->size = size;
    head = tail = 0;
    counter = 0;
    buffer = (char*) MemAllocator::getInstance().malloc(size * sizeof(char));
    _sem::sem_open(&itemAvailable, 0);
    _sem::sem_open(&spaceAvailable, size);
}

void BoundedBuffer::putChar(char c) {
    spaceAvailable->semWait();
    buffer[head] = c;
    head = (head + 1) % size;
    counter++;
    itemAvailable->semSignal();
}

char BoundedBuffer::getChar() {
    itemAvailable->semWait();
    char c = buffer[tail];
    tail = (tail + 1) % size;
    counter--;
    spaceAvailable->semSignal();
    return c;
}

BoundedBuffer::~BoundedBuffer() {
    MemAllocator::getInstance().mem_free((void*) buffer);
    itemAvailable->semClose();
    spaceAvailable->semClose();
}