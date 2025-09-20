#include "../h/syscall_cpp.hpp"
#include "../h/syscall_c.hpp"


void* operator new(size_t size) {
    return mem_alloc(size);
}

void operator delete(void* ptr) {
    mem_free(ptr);
}

Thread::Thread(void (*body)(void *), void *arg) {
    this->body = body;
    this->arg = arg;
}

Thread::~Thread() { }

int Thread::start() {
    thread_create(&myHandle, body, arg);
    return 0;
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep(time_t timeout) {
    return time_sleep(timeout);
}

Thread::Thread() {
    body = &threadWrapper;
    arg = this;
}

void Thread::threadWrapper(void *arg) {
    ((Thread*) arg)->run();
}

Semaphore::Semaphore(unsigned init) {
    sem_open(&myHandle, init);
}

Semaphore::~Semaphore() {
    sem_close(myHandle);
}

int Semaphore::wait() {
    return sem_wait(myHandle);
}

int Semaphore::signal() {
    return sem_signal(myHandle);
}

void PeriodicThread::terminate() {
    thread_exit();
}

PeriodicThread::PeriodicThread(time_t period) {
    this->period = period;
}

void PeriodicThread::run() {
    while(1) {
        this->periodicalActivation();
        time_sleep(period);
    }
}

char Console::getc() {
    return ::getc();
}

void Console::putc(char c) {
    ::putc(c);
}
