#include "../h/semaphore.hpp"

_sem::_sem(unsigned value) {
    this->value = (int) value;
    closed = false;
}

int _sem::sem_open(_sem** handle, unsigned value) {
    *handle = new _sem(value);
    return 0;
}

int _sem::semClose() {
    if (closed)
        return -1;
    else {
        closed = true;
        value = 0;
        TCB* thread = blockedQueue.removeFirst();
        while (thread) {
            if (thread->getThreadStatus() != TCB::WAITING)
                return -1;
            thread->ready();
        }
        return 0;
    }
}

int _sem::semWait() {
    if (closed)
        return -1;
    value--;
    if (value < 0) {
        TCB::running->setThreadStatus(TCB::WAITING);
        blockedQueue.addLast(TCB::running);
        thread_dispatch();
    }
    if (closed)
        return -1;
    else
        return 0;
}

int _sem::semSignal() {
    if (closed)
        return -1;
    value++;
    if (value <= 0) {
        TCB* thread = blockedQueue.removeFirst();
        thread->ready();
        if (thread == TCB::kernel) {
            thread->setThreadStatus(TCB::READY);
            Scheduler::getInstance().put(thread);
        }
    }
    return 0;
}
