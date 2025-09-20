#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "TCB.hpp"
#include "list.hpp"

class _sem {
public:
    static int sem_open(_sem** handle, unsigned value);
    int semClose();
    int semWait();
    int semSignal();

private:
    _sem(unsigned value);

    int value;
    bool closed;
    List<TCB> blockedQueue;
};


#endif