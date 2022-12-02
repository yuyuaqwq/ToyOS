#ifndef THREAD_THREAD_H_
#define THREAD_THREAD_H_

#include "lib/stdint.h"

typedef void ThreadFunc(void*);

typedef enum _TaskStatus {
    kTaskRunning,
    kTaskReady,
    kTaskBlocked,
    kTaskWaiting,
    kTaskHanging,
    kTaskDied,
} TaskStatus;

/*
* 中断上下文环境
*/
typedef struct _IntrStack {
    uint32 vecNo;
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 espDummy;
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;
    uint32 gs;
    uint32 fs;
    uint32 es;
    uint32 ds;

// 从低特权级进入高特权级自动压入
    uint32 errCode;
    void (*eip)(void);
    uint32 cs;
    uint32 eflags;
    uint32* esp;
    uint32 ss;
} IntrStack;

typedef struct _ThreadStack {
    uint32 ebp;
    uint32 ebx;
    uint32 edi;
    uint32 esi;

    void (*eip)(ThreadFunc* func, void* funcArg);

    void (*unusedRetAddr);
    ThreadFunc* function;
    void* funcArg;
} ThreadStack;


typedef struct _TaskStruct {
    uint32* selfKStack;
    TaskStatus status;
    uint8 priority;
    char name[16];
    uint32 stackMagic;
} TaskStruct;


#endif // THREAD_THREAD_H_