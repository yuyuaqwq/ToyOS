#include "tss.h"

#include "lib/stdint.h"
#include "lib/string.h"
#include "lib/kernel/print.h"
#include "kernel/memory.h"
#include "kernel/global.h"
#include "thread/thread.h"

typedef struct _Tss {
    uint32_t backlink;
    uint32_t* esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t (*eip)(void);
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint32_t trace;
    uint32_t ioBase;
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
    gsTss.esp0 = (uint32_t*)((uint32_t)pThread + PG_SIZE);
}

/*
* 构建段描述符
*/
static GdtDesc MakeGdtDesc(uint32_t* descAddr, uint32_t limit, uint32_t attr) {
    uint32_t descBase = (uint32_t)descAddr;
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
    uint32_t tssSize = sizeof(Tss);
    memset(&gsTss, 0, tssSize);
    gsTss.ss0 = SELECTOR_K_STACK;
    gsTss.ioBase = tssSize;

    // GDT[4]TSS
    *((GdtDesc*)0xc0000920) = MakeGdtDesc((uint32_t*)&gsTss, tssSize - 1, TSS_ATTR);
    // GDT[5]用户数据段
    *((GdtDesc*)0xc0000928) = MakeGdtDesc((uint32_t*)0, 0xfffff, GDT_CODE_ATTR);
    // GDT[6]用户代码段
    *((GdtDesc*)0xc0000930) = MakeGdtDesc((uint32_t*)0, 0xfffff, GDT_DATA_ATTR);

    // 需要修改界限，重新加载GDT
    uint64_t gdtOperand = ((8 * 7 - 1) | ((uint64_t )(uint32_t)0xc0000900 << 16));
    asm volatile("lgdt %0" : : "m"(gdtOperand));

    asm volatile("ltr %w0" : : "r"(SELECTOR_TSS));      // 加载Tss
    PutStr("TssInit and ltr done\n");
}