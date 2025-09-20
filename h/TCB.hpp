#ifndef TCB_HPP
#define TCB_HPP

#include "scheduler.hpp"
#include "../lib/hw.h"
#include "syscall_c.hpp"

class TCB {
public:
    enum Thread_Status { CREATED, READY, RUNNING, WAITING, SLEEPING, FINISHED };

    ~TCB() { mem_free(stack); }

    static void init();

    bool isFinished() const { return status == FINISHED; }

    void setThreadStatus (Thread_Status status) { this->status = status; };
    Thread_Status getThreadStatus() const { return this->status; }

    time_t getTimeout() const { return timeout; }
    void decreaseTimeout() { timeout--; }

    time_t getTimeSlice() const { return timeSlice; }
    void setTimeSlice(time_t timeSlice) { this->timeSlice = timeSlice; }

    bool isSystemThread() const { return systemThread; }

    using Body = void (*)(void *);

    static int createThread(TCB* handle ,Body body, void* arg, uint64* stack);
    static void dispatch();
    static int exit();
    static int sleep(time_t time);
    void ready();

private:
    TCB(Body body, void *arg, uint64 *stack) : body(body),
    stack(body != nullptr ? (uint64 *) (stack + DEFAULT_STACK_SIZE): nullptr),
        context({ body != nullptr ? (uint64) &threadWrapper : 0, stack != nullptr ? (uint64) ((char*) stack + DEFAULT_STACK_SIZE) : 0}),
        timeSlice((time_t)DEFAULT_TIME_SLICE),
        timeout(0),
        systemThread(false),
        finished(false),
        status(CREATED) {
            this->arg = arg;
    }

    struct Context {
        uint64 ra;
        uint64 sp;
    };

    Body body;
    void *arg;
    uint64 *stack;
    Context context;
    time_t timeSlice;
    time_t timeout;
    bool systemThread;
    bool finished;
    Thread_Status status;

    static time_t timeSliceCounter;

    static TCB *running;
    static TCB *kernel;
    static TCB *putThread;
    static TCB *idle;

    friend class Riscv;
    friend class _sem;

    static void threadWrapper();
    static void idleWrapper(void* arg);
    static void outputWrapper(void *arg);

    static void contextSwitch(Context *oldContext, Context *runningContext);
};

#endif