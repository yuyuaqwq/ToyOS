#include "kernel/init.h"
#include "kernel/debug.h"
#include "lib/kernel/print.h"
#include "thread/thread.h"

// 要让_main作为第一个函数实现
void kThread_a(void* arg);
void kThread_b(void* arg);

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();

    ThreadStart("kThread_a", 31, kThread_a, "argA ");
    ThreadStart("kThread_b", 31, kThread_b, "argB ");

    IntrEnable();
    while (1) {
        PutStr("Main ");
        int i = 0;
        while(i < 10000000) {
            i++;
        }
    }

    while(1);
    return 0;
}

void kThread_a(void* arg) {
    char* para = arg;
    while(1) {
        PutStr(para);
        int i = 0;
        while(i < 10000000) {
            i++;
        }
    }
}


void kThread_b(void* arg) {
    char* para = arg;
    while(1) {
        PutStr(para);
        int i = 0;
        while(i < 10000000) {
            i++;
        }
    }
}