#include "kernel/init.h"
#include "kernel/debug.h"
#include "device/console.h"
#include "thread/thread.h"
#include "userprog/process.h"
#include "device/ioqueue.h"
#include "device/keyboard.h"

// 要让_main作为第一个函数实现
void kThread_a(void* arg);
void kThread_b(void* arg);

void uProg_a(void);
void uProg_b(void);

int test_var_a = 0, test_var_b = 0;

// nm指令可以查看程序符号

int _main(void) {
    PutStr("I am kernel\n");
    InitAll();

    ThreadStart("kThread_a", 31, kThread_a, " B_");
    ThreadStart("kThread_b", 31, kThread_b, " A_");

    ProcessExecute(uProg_a, "uProg_a");
    //ProcessExecute(uProg_b, "uProg_b");

    IntrEnable();
    while(1);
    return 0;
}


void uProg_a(void) {
    // PutStr("uProg_a"); PutStr("\n");
    while(1) {
        test_var_a++;
    }
}

void uProg_b(void) {
    while(1) {
        test_var_b++;
    }
}



void kThread_a(void* arg) {
    char* para = arg;
    while(1) {
        IntrStatus oldStatus = IntrDisable();
        // ConsolePutStr(" v_a:0x");ConsolePutInt(test_var_a);
        if (!IoQueueEmpty(&gKbdBuf)) {
            
            // ConsolePutStr(arg);
            // char byte = IoQueueGetChar(&gKbdBuf);
            // ConsolePutChar(byte);
        }
        IntrSetStatus(oldStatus);
    }
}


void kThread_b(void* arg) {
    char* para = arg;
    while(1) {
        IntrStatus oldStatus = IntrDisable();
        // ConsolePutStr(" v_b:0x");ConsolePutInt(test_var_b);
        if (!IoQueueEmpty(&gKbdBuf)) {
            
            // ConsolePutStr(arg);
            // char byte = IoQueueGetChar(&gKbdBuf);
            // ConsolePutChar(byte);
        }
        IntrSetStatus(oldStatus);
    }
}