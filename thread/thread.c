#include "thread.h"

#include "kernel/memory.h"
#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/interrupt.h"

#include "userprog/process.h"

/*
* 内核线程初次被调度时执行的中转函数，从SwitchTo返回到此处
*/
static void KernelThread(ThreadFunc* function, void* funcArg) {
    // 执行function前开启中断，避免中断被屏蔽而无法调度其它线程
    // 因为当前是新线程，并不是从其他时钟中断被切出的，并不会返回到时钟中断例程处，而是走到这里
    IntrEnable();
    function(funcArg);
}

TaskStruct* gMainThread;
List gThreadReadyList;
List gThreadAllList;

// 当前正在运行的线程
static ListElem* gsThreadTag;

extern void SwitchTo(TaskStruct* cur, TaskStruct* next);

/*
* 调度线程
*/
void Schedule(void) {
    TaskStruct* cur = RunningThread();

    //ElemPrint(&cur->generalTag, "----------curThread");
    PutStr("curThreadName:"); PutStr(cur->name); PutStr("\n");
    //ListPrint(&gThreadAllList, "Schedule.all");
    //ListPrint(&gThreadReadyList, "Schedule.ready1");
    
    ASSERT(IntrGetStatus() == kIntrOff);

    if (cur->status == kTaskRunning) {
        // 当前线程的时间片耗尽，放到就绪队列尾部
        ASSERT(!ElemFind(&gThreadReadyList, &cur->generalTag));
        ListAppend(&gThreadReadyList, &cur->generalTag);
        cur->ticks = cur->priority;     // 重置时间片
        cur->status = kTaskReady;
    } else {
        
    }

    ASSERT(!ListEmpty(&gThreadReadyList));

    // 从就绪队列弹出第一个线程并切换到目标环境，完成线程切换
    gsThreadTag = NULL;
    gsThreadTag = ListPop(&gThreadReadyList);
    TaskStruct* next = Elem2Entry(TaskStruct, generalTag, gsThreadTag);
    next->status = kTaskRunning;
    ProcessActivate(next);
    PutStr("newThreadName:"); PutStr(next->name); PutStr("\n");
    SwitchTo(cur, next);        // 切换线程
}

/*
* 获取当前线程的PCB(当前栈底地址)
*/
TaskStruct* RunningThread(void) {
    uint32 esp;
    asm("mov %%esp, %0" : "=g"(esp));
    return (TaskStruct*)(esp & 0xfffff000);     // 栈与PCB在同一页面，故取页面基址
}

/*
* 初始化线程栈
*/
void ThreadCreate(TaskStruct* pThread, ThreadFunc function, void* funcArg) {
    pThread->selfKStack -= sizeof(IntrStack);       // 栈顶预留中断栈空间
    pThread->selfKStack -= sizeof(ThreadStack);     // 栈顶预留线程栈空间

    // 待执行函数和参数放到ThreadStack相应的位置
    ThreadStack* kThreadStack = (ThreadStack*)pThread->selfKStack;
    
    // 传递给KernelThread的参数
    kThreadStack->function = function;
    kThreadStack->funcArg = funcArg;

    // 寄存器环境，在切换时恢复
    kThreadStack->eip = KernelThread;       // Switch函数中ret到此地址
    kThreadStack->ebp = kThreadStack->ebx = kThreadStack->esi = kThreadStack->edi = 0;
}

/*
* 初始化线程基本信息
*/
void ThreadInit(TaskStruct* pThread, const char* name, int prio) {
    memset(pThread, 0, sizeof(*pThread));
    strcpy(pThread->name, name);
    if (pThread == gMainThread) {
        // 如果被初始化的线程是main线程，直接设置为运行状态，因为当前执行流一定是main线程
        pThread->status = kTaskRunning;
    } else {
        // 否则挂入调度线程队列
        pThread->status = kTaskReady;
    }

    // PCB同一页面的高地址即是该线程的内核栈空间，这个是栈顶地址
    pThread->selfKStack = (uint32*)((uint32)pThread + PG_SIZE);

    pThread->priority = prio;
    pThread->ticks = prio;
    pThread->elapsedTicks = 0;
    pThread->pgDir = NULL;
    pThread->stackMagic = 'THRE';       // 魔数，用于检查栈是否将PCB覆盖
}

/*
* 启动一个线程
*/
TaskStatus* ThreadStart(const char* name, int prio, ThreadFunc function, void* funcArg) {
    TaskStruct* pThread = GetKernelPages(1);        // 分配PCB和内核栈
    ThreadInit(pThread, name, prio);
    ThreadCreate(pThread, function, funcArg);

    // 将其放到线程队列中
    ASSERT(!ElemFind(&gThreadReadyList, &pThread->generalTag));
    ListAppend(&gThreadReadyList, &pThread->generalTag);
    ASSERT(!ElemFind(&gThreadAllList, &pThread->allListTag));
    ListAppend(&gThreadAllList, &pThread->allListTag);
    return pThread;
}

/*
* 阻塞当前线程
*/
void ThreadBlock(TaskStatus stat) {
    ASSERT((stat == kTaskBlocked) || (stat == kTaskWaiting) || (stat == kTaskHanging));
    IntrStatus oldStatus = IntrDisable();
    TaskStruct* curThread = RunningThread();
    curThread->status = stat;
    Schedule();
    IntrSetStatus(oldStatus);
}

/*
* 唤醒线程
*/
void ThreadUnblock(TaskStruct* pThread) {
    IntrStatus oldStatus = IntrDisable();
    ASSERT((pThread->status == kTaskBlocked) || (pThread->status == kTaskWaiting) || (pThread->status == kTaskHanging));
    if (pThread->status != kTaskReady) {
        ASSERT(!ElemFind(&gThreadReadyList, &pThread->generalTag));
        if (ElemFind(&gThreadReadyList, & pThread->generalTag)) {
            PANIC("ThreadUnblock: blocked thread in ready list\n");
        }
        ListPush(&gThreadReadyList, &pThread->generalTag);

        pThread->status = kTaskReady;
    }
    IntrSetStatus(oldStatus);
}






/*
* 构建main线程结构
*/
static void MakeMainThread(void) {
    // 当前esp的范围是0xc009e000 ~ 0xc009efff，得到0xc009e000
    gMainThread = RunningThread();
    ThreadInit(gMainThread, "main", 31);

    // main线程挂入所有线程队列
    ASSERT(!ElemFind(&gThreadAllList, &gMainThread->allListTag));
    ListAppend(&gThreadAllList, &gMainThread->allListTag);
}

/*
* 初始化线程环境，当前执行流为main线程
*/
void ThreadEnvirInit(void) {
    
    PutStr("ThreadInitEnvir start\n");
    ListInit(&gThreadAllList);
    ListInit(&gThreadReadyList);

    MakeMainThread();
    PutStr("ThreadInitEnvir done\n");
}