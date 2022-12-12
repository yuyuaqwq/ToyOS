#include "process.h"

#include "lib/string.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "kernel/debug.h"
#include "thread/thread.h"
#include "userprog/tss.h"
#include "device/console.h"



extern void IntrExit(void);

/*
* 用户进程起始函数，从内核线程起始函数KernelThread中调用
*/
void StartProcess(void* fileName_) {
    void* function = fileName_;
    TaskStruct* cur = RunningThread();
    cur->selfKStack += sizeof(ThreadStack);
    IntrStack* procStack = (IntrStack*)cur->selfKStack;
    PutInt(procStack); PutStr("\n");
    // 构建栈，通过iretd降权到r3
    procStack->edi = procStack->esi = procStack->ebp = procStack->espDummy = 0;
    procStack->ebx = procStack->edx = procStack->ecx = procStack->eax = 0;
    procStack->gs = 0;
    procStack->ds = procStack->es = procStack->fs = SELECTOR_U_DATA;
    procStack->eip = function;
    procStack->cs = SELECTOR_U_CODE;
    procStack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    procStack->esp = (void*)((uint32)GetAPage(kPfUser, USER_STACK3_VADDR) + PG_SIZE);       // 分配用户进程栈
    procStack->ss = SELECTOR_U_DATA;
    asm volatile("movl %0, %%esp; jmp IntrExit" : : "g"(procStack) : "memory");
}

/*
* 切换cr3
* 任务为进程时，切换cr3为进程页目录
* 任务为线程时，切换cr3为内核页目录
*/
void PageDirActivate(TaskStruct* pThread) {
    uint32 pageDirPhyAddr = 0x100000;       // 线程统一使用内核页目录
    if (pThread->pgDir != NULL) {
        pageDirPhyAddr = AddrV2P((uint32)pThread->pgDir);
    }

    asm volatile("movl %0, %%cr3" : : "r"(pageDirPhyAddr) : "memory");
    PutInt(pageDirPhyAddr); PutChar('\n');
}

/*
* 切换cr3并更新Tss中的Esp0
*/
void ProcessActivate(TaskStruct* pThread) {
    ASSERT(pThread != NULL);
    PageDirActivate(pThread);
    if (pThread->pgDir != NULL) {
        UpdateTssEsp(pThread);
    }
}

/*
* 创建进程页目录
*/
uint32* CreatePageDir(void) {
    uint32* pageDirVAddr = GetKernelPages(1);
    if (pageDirVAddr == NULL) {
        ConsolePutStr("CreatePageDir: GetKernelPages failed!");
        return NULL;
    }

    // 进程共用内核空间，映射到相同的页表中
    memcpy((void*)((uint32)pageDirVAddr + 0x300 * 4), (const void*)(0xfffff000 + 0x300 * 4), 0x100 * 4);
    uint32 newPageDirPhyAddr = AddrV2P((uint32)pageDirVAddr);
    pageDirVAddr[1023] = newPageDirPhyAddr | PG_US_U |PG_RW_W | PG_P_1;     // 页目录自映射
    return pageDirVAddr;
}

/*
* 创建用户空间虚拟地址管理位图
*/
void CreateUserVAddrBitmap(TaskStruct* userProg) {
    userProg->userprogVAddr.vAddrStart = USER_VADDR_START;
    uint32 bitmapPgCnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    userProg->userprogVAddr.vAddrBitmap.bits = GetKernelPages(bitmapPgCnt);
    userProg->userprogVAddr.vAddrBitmap.btmpByteLen = (0xc0000000 - USER_VADDR_START) / PG_SIZE/ 8;
    BitmapInit(&userProg->userprogVAddr.vAddrBitmap);
}

/*
* 启动新进程
*/
void ProcessExecute(void* filename, char* name) {
    // 进程与线程的区别并不明确，实际上都可以叫做进程，也都可以叫做线程
    TaskStruct* pThread = GetKernelPages(1);        // 进程有自己的页目录
    ThreadInit(pThread, name, DEFAULT_PRIO);
    CreateUserVAddrBitmap(pThread);     // 进程需要管理自己的用户空间地址
    ThreadCreate(pThread, StartProcess, filename);
    pThread->pgDir = CreatePageDir();


    // 添加到调度链表中
    IntrStatus oldStatus = IntrDisable();
    ASSERT(!ElemFind(&gThreadReadyList, &pThread->generalTag));
    ListAppend(&gThreadReadyList, &pThread->generalTag);

    ASSERT(!ElemFind(&gThreadAllList, &pThread->allListTag));
    ListAppend(&gThreadAllList, &pThread->allListTag);

    IntrSetStatus(oldStatus);
}