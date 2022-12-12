#include "tss.h"

#include "lib/stdint.h"
#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/memory.h"
#include "kernel/global.h"
#include "thread/thread.h"

typedef struct _Tss {
    uint32 backlink;
    uint32* esp0;
    uint32 ss0;
    uint32 esp1;
    uint32 ss1;
    uint32 esp2;
    uint32 ss2;
    uint32 cr3;
    uint32 (*eip)(void);
    uint32 eflags;
    uint32 eax;
    uint32 ecx;
    uint32 edx;
    uint32 ebx;
    uint32 esp;
    uint32 ebp;
    uint32 esi;
    uint32 edi;
    uint32 es;
    uint32 cs;
    uint32 ss;
    uint32 ds;
    uint32 fs;
    uint32 gs;
    uint32 ldt;
    uint32 trace;
    uint32 ioBase;
} Tss;

/*
* 全局Tss结构，仅需要Tss中的esp0和ss0，提权时使用
*/
static Tss gsTss;

/*
* 更新全局Tss的esp0字段为内核栈底
*/
void UpdateTssEsp(TaskStruct* pThread) {
    // 切换任务也要切换Tss指向的内核栈
    gsTss.esp0 = (uint32*)((uint32)pThread + PG_SIZE);
}

/*
* 构建段描述符
*/
static GdtDesc MakeGdtDesc(uint32* descAddr, uint32 limit, uint32 attr) {
    uint32 descBase = (uint32)descAddr;
    GdtDesc desc;
    
    desc.limitLowWord = limit & 0x0000ffff;
    desc.baseLowWord = descBase & 0x0000ffff;

    desc.highDword = 0;
    desc.highDword |= attr;
    desc.highDword |= limit & 0x000f0000;
    desc.highDword |= (descBase >> 16) & 0xff;
    desc.highDword |= descBase & 0xff000000;
    return desc;
}

/*
* Tss初始化
*/
void TssInit(void) {
    PutStr("TssInit start\n");
    uint32 tssSize = sizeof(Tss);
    memset(&gsTss, 0, tssSize);
    gsTss.ss0 = SELECTOR_K_STACK;
    gsTss.ioBase = tssSize;

    // GDT[4]TSS
    *((GdtDesc*)0xc0000920) = MakeGdtDesc((uint32*)&gsTss, tssSize - 1, TSS_ATTR);
    // GDT[5]用户数据段
    *((GdtDesc*)0xc0000928) = MakeGdtDesc((uint32*)0, 0xfffff, GDT_CODE_ATTR);
    // GDT[6]用户代码段
    *((GdtDesc*)0xc0000930) = MakeGdtDesc((uint32*)0, 0xfffff, GDT_DATA_ATTR);

    // 需要修改界限，重新加载GDT
    uint64 gdtOperand = ((8 * 7 - 1) | ((uint64)(uint32)0xc0000900 << 16));
    asm volatile("lgdt %0" : : "m"(gdtOperand));

    asm volatile("ltr %w0" : : "r"(SELECTOR_TSS));      // 加载Tss
    PutStr("TssInit and ltr done\n");
}