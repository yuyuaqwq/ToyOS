#include "kernel/init.h"
#include "kernel/debug.h"
#include "device/console.h"
#include "thread/thread.h"

#include "device/ioqueue.h"
#include "device/keyboard.h"

// 要让_main作为第一个函数实现
void kThread_a(void* arg);
void kThread_b(void* arg);

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();

    ThreadStart("kThread_a", 31, kThread_a, " B_");
    ThreadStart("kThread_b", 31, kThread_b, " A_");

    IntrEnable();
    while(1);
    return 0;
}

void kThread_a(void* arg) {
    char* para = arg;
    while(1) {
        IntrStatus oldStatus = IntrDisable();
        if (!IoQueueEmpty(&gKbdBuf)) {
            ConsolePutStr(arg);
            char byte = IoQueueGetChar(&gKbdBuf);
            ConsolePutChar(byte);
        }
        IntrSetStatus(oldStatus);
    }
}


void kThread_b(void* arg) {
    char* para = arg;
    while(1) {
        IntrStatus oldStatus = IntrDisable();
        if (!IoQueueEmpty(&gKbdBuf)) {
            ConsolePutStr(arg);
            char byte = IoQueueGetChar(&gKbdBuf);
            ConsolePutChar(byte);
        }
        IntrSetStatus(oldStatus);
    }
}