#include "thread.h"

#include "kernel/memory.h"
#include "lib/string.h"

static void KernelThread(ThreadFunc* function, void* funcArg) {
    function(funcArg);
}

/*
* 填充线程结构字段
*/
void ThreadCreate(TaskStruct* pThread, ThreadFunc function, void* funcArg) {
    pThread->selfKStack -= sizeof(IntrStack);       // 中断栈空间
    pThread->selfKStack -= sizeof(ThreadStack);     // 线程栈空间

    // 为线程栈空间填充数据
    ThreadStack* kThreadStack = (ThreadStack*)pThread->selfKStack;
    kThreadStack->eip = KernelThread;
    kThreadStack->function = function;
    kThreadStack->funcArg = funcArg;
    kThreadStack->ebp = kThreadStack->ebx = kThreadStack->esi = kThreadStack->edi = 0;
}

/*
* 初始化线程结构
*/
void ThreadInit(TaskStruct* pThread, char* name, int prio) {
    memset(pThread, 0, sizeof(*pThread));
    strcpy(pThread->name, name);
    pThread->status = kTaskRunning;
    pThread->priority = prio;
    
    pThread->selfKStack = (uint32*)((uint32)pThread + PG_SIZE);     // 栈指向当前页面尾部
    pThread->stackMagic = 'THRE';       // 魔数，用于检查栈是否覆写任务结构体
}

/*
* 启动一个线程
*/
TaskStatus* ThreadStart(char* name, int prio, ThreadFunc function, void* funcArg) {
    TaskStruct* pThread = GetKernelPages(1);
    ThreadInit(pThread, function, funcArg);
    ThreadCreate(pThread, function, funcArg);
    asm volatile ("movl %0, %%esp; pop %%ebp; pop %% ebx; pop %%edi; pop %%esi; ret" : : "g"(pThread->selfKStack) : "memory");
    return pThread;
}