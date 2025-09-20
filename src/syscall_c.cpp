#include "../h/syscall_c.hpp"
#include "../h/riscv.hpp"

void* mem_alloc(size_t size) {
    __asm__ volatile ("mv a1, %0" :: "r"(size));
    __asm__ volatile ("mv a0, %0" :: "r"(MEM_ALLOC));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (void*) retValue;
}

int mem_free(void* ptr) {
    if (!ptr) return -1;
    __asm__ volatile ("mv a1, %0" :: "r"(ptr));
    __asm__ volatile ("mv a0, %0" :: "r"(MEM_FREE));
    __asm__ volatile ("ecall");
    return (int) 0;
}

size_t mem_get_free_space() {
    __asm__ volatile ("mv a0, %0" :: "r"(MEM_GET_FREE_SPACE));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (size_t) retValue;
}

size_t mem_get_largest_free_block() {
    __asm__ volatile ("mv a0, %0" :: "r"(MEM_GET_LARGEST));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (size_t) retValue;
}

int thread_create (thread_t* handle, void(*start_routine) (void*), void* arg) {
    uint64* stack = (uint64*) mem_alloc(DEFAULT_STACK_SIZE);
    __asm__ volatile ("mv a4, %0" :: "r"(stack));
    __asm__ volatile ("mv a3, %0" :: "r"(arg));
    __asm__ volatile ("mv a2, %0" :: "r"(start_routine));
    __asm__ volatile ("mv a1, %0" :: "r"(handle));
    __asm__ volatile ("mv a0, %0" :: "r"(THREAD_CREATE));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}
int thread_exit () {
    __asm__ volatile ("mv a0, %0" :: "r"(THREAD_EXIT));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}

void thread_dispatch () {
    __asm__ volatile ("mv a0, %0" :: "r"(THREAD_DISPATCH));
    __asm__ volatile ("ecall");
}

int sem_open(sem_t* handle, unsigned init) {
    __asm__ volatile ("mv a2, %0" :: "r"(init));
    __asm__ volatile ("mv a1, %0" :: "r"(handle));
    __asm__ volatile ("mv a0, %0" :: "r"(SEM_OPEN));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}
int sem_close (sem_t handle) {
    __asm__ volatile ("mv a1, %0" :: "r"(handle));
    __asm__ volatile ("mv a0, %0" :: "r"(SEM_CLOSE));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}

int sem_wait (sem_t id) {
    __asm__ volatile ("mv a1, %0" :: "r"(id));
    __asm__ volatile ("mv a0, %0" :: "r"(SEM_WAIT));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}

int sem_signal (sem_t id) {
    __asm__ volatile ("mv a1, %0" :: "r"(id));
    __asm__ volatile ("mv a0, %0" :: "r"(SEM_SIGNAL));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}

int time_sleep (time_t time) {
    __asm__ volatile ("mv a1, %0" :: "r"(time));
    __asm__ volatile ("mv a0, %0" :: "r"(TIME_SLEEP));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (int) retValue;
}

char getc () {
    __asm__ volatile ("mv a0, %0" :: "r"(GET_C));
    __asm__ volatile ("ecall");
    uint64 volatile retValue;
    __asm__ volatile ("mv %0, a0" : "=r"(retValue));
    return (char) retValue;
}

void putc (char c) {
    __asm__ volatile ("mv a1, %0" :: "r"(c));
    __asm__ volatile ("mv a0, %0" :: "r"(PUT_C));
    __asm__ volatile ("ecall");
}