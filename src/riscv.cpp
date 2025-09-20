#include "../h/riscv.hpp"
#include "../lib/console.h"
#include "../h/TCB.hpp"
#include "../h/memAllocator.hpp"
#include "../h/semaphore.hpp"
#include "../h/syscall_c.hpp"
#include "../test/printing.hpp"

using Body = void(*)(void*);

BoundedBuffer* Riscv::bufferIn = nullptr;
BoundedBuffer* Riscv::bufferOut = nullptr;

void Riscv::init() {
    bufferIn = new BoundedBuffer(300);
    bufferOut = new BoundedBuffer(300);
}

void Riscv::popSppSpie() {
    __asm__ volatile ("csrw sepc, ra");
    __asm__ volatile ("sret");
}

void Riscv::restorePrivilege() {
    if (TCB::running->isSystemThread())
        ms_sstatus(SSTATUS_SPP);
    else {
        ms_sstatus(SSTATUS_SPIE);
        mc_sstatus(SSTATUS_SPP);
    }
}

void Riscv::handleSupervisorTrap() {
    uint64 volatile scause = r_scause();
    uint64 volatile sstatus = r_sstatus();
    uint64 volatile sepc = r_sepc();
    if (scause == SUPER_ECALL || scause == USER_ECALL) {
        //no interrupt, cause code: environment call from S mode (9)
        sepc = sepc + 4;
        uint64 volatile opcode;
        __asm__ volatile ("ld %0, 80(s0)" : "=r"(opcode));
        switch (opcode) {
            case MEM_ALLOC: {
                uint64 size;
                void *ptr;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(size));
                ptr = MemAllocator::getInstance().malloc((size_t) size);
                __asm__ volatile ("mv a0, %0" :: "r"(ptr));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case MEM_FREE: {
                uint64 volatile ptr;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(ptr));
                MemAllocator::getInstance().mem_free((void *) ptr);
                __asm__ volatile ("mv a0, %0" :: "r"(1));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case MEM_GET_FREE_SPACE: {
                size_t volatile size;
                size = MemAllocator::getInstance().sizeOfFreeSpace();
                __asm__ volatile ("mv a0, %0" :: "r"(size));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case MEM_GET_LARGEST: {
                size_t volatile size;
                size = MemAllocator::getInstance().findBiggestFreeBlock();
                __asm__ volatile ("mv a0, %0" :: "r"(size));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case THREAD_CREATE: {
                thread_t volatile handle;
                Body volatile start_routine;
                void* volatile arg;
                uint64* volatile stack;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(handle));
                __asm__ volatile ("ld %0, 96(s0)" : "=r"(start_routine));
                __asm__ volatile ("ld %0, 104(s0)" : "=r"(arg));
                __asm__ volatile ("ld %0, 112(s0)" : "=r"(stack));
                int retValue = TCB::createThread((thread_t) handle, (Body) start_routine, (void*) arg, (uint64*) stack);
                __asm__ volatile ("mv a0, %0" :: "r"(retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case THREAD_EXIT: {
                uint64 volatile retValue;
                retValue = TCB::exit();
                __asm__ volatile ("mv a0, %0" :: "r"(retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case THREAD_DISPATCH: {
                TCB::dispatch();
                break;
            }
            case SEM_OPEN: {
                sem_t *volatile handle;
                unsigned volatile init;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(handle));
                __asm__ volatile ("ld %0, 96(s0)" : "=r"(init));
                int retValue = _sem::sem_open((sem_t *)handle, (unsigned) init);
                __asm__ volatile ("mv a0, %0" :: "r" (retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case SEM_CLOSE: {
                sem_t volatile id;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(id));
                int retValue;
                if (id == nullptr)
                    retValue = -2;
                else
                    retValue = ((sem_t) id)->semClose();
                __asm__ volatile ("mv a0, %0" :: "r" (retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case SEM_WAIT: {
                sem_t volatile id;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(id));
                int retValue;
                if (id == nullptr)
                    retValue = -2;
                else
                    retValue = ((sem_t) id)->semWait();
                __asm__ volatile ("mv a0, %0" :: "r" (retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case SEM_SIGNAL: {
                sem_t volatile id;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(id));
                int retValue;
                if (id == nullptr)
                    retValue = -2;
                else
                    retValue = ((sem_t) id)->semSignal();
                __asm__ volatile ("mv a0, %0" :: "r" (retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case TIME_SLEEP: {
                time_t volatile timeout;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(timeout));
                int retValue = (TCB::sleep((time_t) timeout));
                __asm__ volatile ("mv a0, %0" :: "r" (retValue));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case GET_C: {
                char volatile c;
                c = bufferIn->getChar();
                __asm__ volatile("mv a0, %0" :: "r"(c));
                __asm__ volatile ("sd a0, 80(s0)");
                break;
            }
            case PUT_C: {
                char volatile c;
                __asm__ volatile ("ld %0, 88(s0)" : "=r"(c));
                bufferOut->putChar(c);
                break;
            }
        }
        w_sepc(sepc);
        w_sstatus(sstatus);
    }
    else if (scause == SOFTWARE_I) {
        //is interrupt, cause code, supervisor software interrupt (timer)
        TCB::timeSliceCounter++;
        Scheduler::getInstance().tick();
        mc_sip(SIP_SSIP);
        if (TCB::timeSliceCounter >= TCB::running->timeSlice) {
            TCB::dispatch();
        }
        w_sepc(sepc);
        w_sstatus(sstatus);
        mc_sip(SIP_SSIP);
    }
    else if (scause == EXTERNAL_I) {
        //interrupt yes, cause code: supervisor external interrupt (console)
        int irq = plic_claim();
        if(irq == CONSOLE_IRQ) {
            char volatile status = (* (char*) CONSOLE_STATUS);
            while(status & CONSOLE_RX_STATUS_BIT) {
                char volatile c = (*(char*) CONSOLE_RX_DATA);
                bufferIn->putChar(c);
                status = (*(char*) CONSOLE_STATUS);
            }
        }
        plic_complete(irq);
        w_sepc(sepc);
        w_sstatus(sstatus);
    }
    else {
        //unexpected trap cause
        uint64 scause= r_scause();
        uint64 stval= r_stval();
        uint64 sepc= r_sepc();
        printString("scause:");
        printInt(scause, 16, 0);
        printString("\n");
        printString("stval:");
        printInt(stval, 16, 0);
        printString("\n");
        printString("sepc:");
        printInt(sepc, 16, 0);
        printString("\n");
    }
}

