
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

static Tss gsTss;

void UpdateTssEsp(TaskStruct* pThread) {
    // 切换任务也要切换Tss指向的内核栈
    gsTss.esp0 = (uint32*)((uint32)pThread + PG_SIZE);
}

static GdtDesc MakeGdtDesc(uint32* descAddr, uint32 limit, uint8 attrLow, uint8 attrHigh) {
    uint32 descBase = (uint32)descAddr;
    GdtDesc desc;
    desc.limitLowWord = limit & 0x0000ffff;
    desc.baseLowWord = descBase & 0x0000ffff;
    desc.baseMidByte = ((descBase & 0x00ff0000) >> 16);
    desc.attrLowByte = (uint8)attrLow;
    desc.limitHighAttrHigh = (((limit & 0x000f0000) >> 16) + (uint8)attrHigh);
    desc.baseHighByte = descBase >> 24;
    return desc;
}

void TssInit() {
    PutStr("TssInit start\n");
    uint32 tssSize = sizeof(Tss);
    memset(&gsTss, 0, tssSize);
    gsTss.ss0 = SELECTOR_K_STACK;
    gsTss.ioBase = tssSize;

    // GDT[4]TSS
    *((GdtDesc*)0xc0000920) = MakeGdtDesc((uint32*)&gsTss, tssSize - 1, TSS_ATTR_LOW, TSS_ATTR_HIGH);
    // GDT[5]用户数据段
    *((GdtDesc*)0xc0000928) = MakeGdtDesc((uint32*)0, 0xfffff, GDT_CODE_ATTR_LOW_DPL3, GDT_ATTR_HIGH);
    // GDT[6]用户代码段
    *((GdtDesc*)0xc0000930) = MakeGdtDesc((uint32*)0, 0xfffff, GDT_DATA_ATTR_LOW_DPL3, GDT_ATTR_HIGH);


    uint64 gdtOperand = ((8 * 7 - 1) | ((uint64)(uint32)0xc0000900 << 16));

    asm volatile("lgdt %0" : : "m"(gdtOperand));
    asm volatile("ltr %w9" : : "r"(SELECTOR_TSS));
    PutStr("TssInit and ltr done\n");

}