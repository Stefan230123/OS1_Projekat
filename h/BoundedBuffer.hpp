#ifndef BOUNDED_BUFFER_H
#define BOUNDED_BUFFER_H

#include "semaphore.hpp"

class BoundedBuffer {
public:
    BoundedBuffer(int size);
    ~BoundedBuffer();
    void putChar(char c);
    char getChar();
    bool full() {
        return counter == size;
    }
    int count() {
        return counter;
    }
private:
    char* buffer;
    int size;
    int counter;
    int head;
    int tail;
    _sem* itemAvailable;
    _sem* spaceAvailable;
};

#endif