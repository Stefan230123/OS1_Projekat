#include "../lib/console.h"
#include "../h/memAllocator.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"
#include "../h/semaphore.hpp"
#include "../test/printing.hpp"

extern void userMain();

_sem* waitUser = nullptr;

void userThreadWrapper(void* arg) {
    printString("---------START---------\n");
    userMain();
    printString("----------END----------\n");
    waitUser->semSignal();
}

int main() {
    //initialization of system threads
    Riscv::w_stvec((uint64) &Riscv::supervisorTrap);
    Riscv::init();
    TCB::init();

    //initialization of use thread
    _sem::sem_open(&waitUser, 0);
    thread_t user = nullptr;
    TCB::createThread(user, userThreadWrapper, nullptr, (uint64*) MemAllocator::getInstance().malloc(DEFAULT_STACK_SIZE));
    TCB::dispatch();
    waitUser->semWait();

    while(Riscv::bufferOut->count() > 0) {
        TCB::dispatch();
    }

    //end of program
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    //stopping emulator
    volatile uint64 *stop_address = (uint64*)0x100000;
    uint64 stop_value = 0x5555;
    asm volatile ("sw %0, 0(%1)":: "r" (stop_value), "r" (stop_address) : "memory");

    return 0;
}