#ifndef THREAD_THREAD_H_
#define THREAD_THREAD_H_

#include "lib/stdint.h"
#include "lib/kernel/list.h"
#include "kernel/memory.h"

typedef void ThreadFunc(void*);

/*
* 线程状态
*/
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
    uint32_t vecNo;
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t espDummy;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

// 从低特权级进入高特权级自动压入
    uint32_t errCode;
    void (*eip)(void);
    uint32_t cs;
    uint32_t eflags;
    uint32_t* esp;
    uint32_t ss;
} IntrStack;


/*
* 线程切换时保存/恢复环境
*/
typedef struct _ThreadStack {
    // 因为是在SwitchTo函数中切换的寄存器环境，只需要备份非易变寄存器，编译器不会生成在调用函数前将不可覆盖值保存到易变寄存器中的指令。
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip)(ThreadFunc* func, void* funcArg);

    void (*unusedRetAddr);
    ThreadFunc* function;
    void* funcArg;
} ThreadStack;

typedef int16_t Pid;
/*
* PCB
*/
typedef struct _TaskStruct {
    uint32_t* selfKStack;     // 内核线程的栈指针，在 切出/切回 时 保存/恢复
    Pid pid;
    TaskStatus status;
    char name[16];
    uint8_t priority;
    uint8_t ticks;        // 每次在处理器上执行的时间嘀嗒数

    uint32_t elapsedTicks;        // 线程所占用的时间嘀嗒数

    ListElem generalTag;        // 在一般线程队列中的节点
    ListElem allListTag;        // 在所有线程队列中的节点

    uint32_t* pgDir;      // 进程页表虚拟地址
    VirAddrPool userprogVAddr;      // 用户进程虚拟地址池
    MemBlockDesc userBlockDesc[MEM_DESC_CNT];       // 用户进程内存块描述符
    uint32_t stackMagic;
} TaskStruct;

void Schedule(void);
TaskStruct* RunningThread(void);
void ThreadInit(TaskStruct* pThread, const char* name, int prio);
void ThreadCreate(TaskStruct* pThread, ThreadFunc function, void* funcArg);
TaskStatus* ThreadStart(const char* name, int prio, ThreadFunc function, void* funcArg);
void ThreadEnvirInit(void);

void ThreadYIeld(void);
void ThreadBlock(TaskStatus stat);
void ThreadUnblock(TaskStruct* pThread);

extern TaskStruct* gMainThread;
extern List gThreadReadyList;
extern List gThreadAllList;


#endif // THREAD_THREAD_H_