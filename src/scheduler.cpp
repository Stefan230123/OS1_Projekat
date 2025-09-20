#include "../h/scheduler.hpp"
#include "../h/TCB.hpp"
#include "../h/list.hpp"

Scheduler &Scheduler::getInstance() {
    static Scheduler instance;
    return instance;
}

TCB *Scheduler::get() {
    return readyThreadQueue.removeFirst();
}

void Scheduler::put(TCB *tcb) {
    readyThreadQueue.addLast(tcb);
}

void Scheduler::putToSleep(TCB* thread) {
    sleepingThread.addLast(thread);
}

void Scheduler::tick() {
    List<TCB>::Elem* curr = sleepingThread.head;
    List<TCB>::Elem* prev = nullptr;

    while (curr != nullptr) {
        curr->data->decreaseTimeout();
        if (curr->data->getTimeout() == 0) {
            List<TCB>::Elem* toDelete = curr;
            List<TCB>::Elem* next = curr->next;

            if (prev) {
                prev->next = next;
            } else {
                sleepingThread.head = next;
            }

            if (next == nullptr) {
                sleepingThread.tail = prev;
            }

            curr->data->ready();

            delete toDelete;
            curr = next;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
}
