#include "process.h"

#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "kernel/debug.h"
#include "thread/thread.h"
#include "userprog/tss.h"
#include "device/console.h"



extern void IntrExit(void);

void StartProcess(void* fileName_) {
    void* function = fileName_;
    TaskStruct* cur = RunningThread();
    cur->selfKStack += sizeof(ThreadStack);
    IntrStack* procStack = (IntrStack*)cur->selfKStack;
    procStack->edi = procStack->esi = procStack->ebp = procStack->espDummy = 0;
    procStack->ebx = procStack->edx = procStack->ecx = procStack->eax = 0;

    procStack->gs = 0;
    procStack->ds = procStack->es = procStack->fs = SELECTOR_U_DATA;
    procStack->eip = function;
    procStack->cs = SELECTOR_U_CODE;
    procStack->eflags = (EFLAGS_IOPL_0 | EFLAGS_MBS | EFLAGS_IF_1);
    procStack->esp = (void*)((uint32)GetAPage(kPfUser, USER_STACK3_VADDR) + PG_SIZE);
    procStack->ss = SELECTOR_U_CODE;
    asm volatile("movl %0, %%esp; jmp IntrExit" : : "g"(procStack) : "memory");
}

void PageDirActivate(TaskStruct* pThread) {
    uint32 pageDirPhyAddr = 0x100000;
    if (pThread->pgDir != NULL) {
        pageDirPhyAddr = AddrV2P((uint32)pThread->pgDir);
    }

    asm volatile("movl %0, %%cr3" : : "r"(pageDirPhyAddr) : "memory");
}

void ProcessActivate(TaskStruct* pThread) {
    ASSERT(pThread != NULL);
    PageDirActivate(pThread);

    if (pThread->pgDir) {
        UpdateTssEsp(pThread);
    }
}

uint32* CreatePageDir(void) {
    uint32* pageDirVAddr = GetKernelPages(1);
    if (pageDirVAddr == NULL) {
        ConsolePutStr("CreatePageDir: GetKernelPages failed!");
        return NULL;
    }

    memcpy((uint32*)((uint32)pageDirVAddr + 0x300 * 4), (uint32*)(0xfffff000 + 0x300 * 4), 1024);

    uint32 newPageDirPhyAddr = AddrV2P((uint32)pageDirVAddr);
    pageDirVAddr[1023] = newPageDirPhyAddr | PG_US_U |PG_RW_W | PG_P_1;
    return pageDirVAddr;
}

void CreateUserVAddrBitmap(TaskStruct* userProg) {
    userProg->userprogVAddr.vAddrStart = USER_VADDR_START;
    uint32 bitmapPgCnt = DIV_ROUND_UP((0xc0000000 - USER_VADDR_START) / PG_SIZE / 8, PG_SIZE);
    userProg->userprogVAddr.vAddrBitmap.bits = GetKernelPages(bitmapPgCnt);
    userProg->userprogVAddr.vAddrBitmap.btmpByteLen = (0xc0000000 - USER_VADDR_START) / PG_SIZE/ 8;
    BitmapInit(&userProg->userprogVAddr.vAddrBitmap);
}

void ProcessExecute(void* filename, char* name) {
    TaskStruct* pThread = GetKernelPages(1);
    ThreadInit(pThread, name, DEFAULT_PRIO);
    CreateUserVAddrBitmap(pThread);
    ThreadCreate(pThread, StartProcess, filename);
    pThread->pgDir = CreatePageDir();

    IntrStatus oldStatus = IntrDisable();
    ASSERT(!ElemFind(&gThreadReadyList, &pThread->generalTag));
    ListAppend(&gThreadReadyList, &pThread->generalTag);

    ASSERT(!ElemFind(&gThreadAllList, &pThread->allListTag));
    ListAppend(&gThreadAllList, &pThread->allListTag);

    IntrSetStatus(oldStatus);
}