#include "kernel/init.h"
#include "kernel/debug.h"
#include "lib/kernel/print.h"
#include "thread/thread.h"

// 要让_main作为第一个函数实现
void kThread_a(void* arg);

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();

    ThreadStart("kThread_a", 31, kThread_a, "argA ");

    while(1);
    return 0;
}

void kThread_a(void* arg) {
    char* para = arg;
    while(1) {
        PutStr(para);
    }
}