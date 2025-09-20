#include "../h/TCB.hpp"
#include "../h/riscv.hpp"
#include "../h/scheduler.hpp"
#include "../lib/mem.h"
#include "../h/syscall_c.hpp"

TCB *TCB::running = nullptr;
TCB *TCB::kernel = nullptr;
TCB *TCB::putThread = nullptr;
TCB *TCB::idle = nullptr;
uint64 TCB::timeSliceCounter = 0;

void TCB::init() {
    if (!kernel) {
        TCB* thread = new TCB(nullptr, nullptr, nullptr);
        kernel = thread;
        running = kernel;
        kernel->systemThread = true;
        kernel->status = RUNNING;
    }
    if (!idle) {
        TCB* thread = new TCB(&idleWrapper, nullptr, (uint64*) MemAllocator::getInstance().malloc(DEFAULT_STACK_SIZE));
        idle = thread;
        idle->systemThread = true;
    }
    if (!putThread) {
        TCB* thread = new TCB(&outputWrapper, nullptr, (uint64*) MemAllocator::getInstance().malloc(DEFAULT_STACK_SIZE));
        putThread = thread;
        putThread->systemThread = true;
        putThread->ready();
    }
}

int TCB::createThread(TCB* handle, Body body, void *arg, uint64 *stack) {
    handle = new TCB(body, arg, stack);
    if (handle) {
        (handle)->ready();
        return 0;
    }
    else
        return -1;
}

void TCB::dispatch() {
    TCB *old = running;
    if (old->status == RUNNING) {
        Scheduler::getInstance().put(old);
    }
    running = Scheduler::getInstance().get();
    if (running)
        running->status = RUNNING;
    else
        running = idle;
    timeSliceCounter = 0;
    Riscv::restorePrivilege();
    TCB::contextSwitch(&old->context, &running->context);
}

int TCB::sleep(time_t sleep) {
    if (TCB::running == kernel || TCB::running->status != RUNNING)
        return -1;
    if (sleep > 0){
        running->status = SLEEPING;
        running->timeout = sleep;
        Scheduler::getInstance().putToSleep(running);
        dispatch();
    }
    return 0;
}

void TCB::threadWrapper() {
    Riscv::restorePrivilege();
    Riscv::popSppSpie();
    if (running->body)
        running->body(running->arg);
    thread_exit();
}

void TCB::idleWrapper(void* arg) {
    while(1)
        Riscv::ms_sstatus(Riscv::SSTATUS_SIE);
}

void TCB::outputWrapper(void* arg) {
    while(1) {
        char volatile status = *((char*) CONSOLE_STATUS);
        while(status & CONSOLE_TX_STATUS_BIT) {
            char c = Riscv::bufferOut->getChar();
            *((char *) CONSOLE_TX_DATA) = c;
            status = *((char*) CONSOLE_STATUS);
        }
    }
}

void TCB::ready() {

        this->status = READY;
        Scheduler::getInstance().put(this);
}

int TCB::exit() {
    if (running->status != RUNNING)
        return -1;
    else {
        running->status = FINISHED;
        dispatch();
        return 0;
    }
}
