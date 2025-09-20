#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "list.hpp"

class TCB;

class Scheduler {
public:
    static Scheduler& getInstance();

    TCB *get();
    void put(TCB *tcb);

    void putToSleep(TCB* thread);
    void tick();

private:
    Scheduler() {};

    List<TCB> readyThreadQueue;
    List<TCB> sleepingThread;
};

#endif