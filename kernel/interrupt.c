#include "interrupt.h"

#include "lib/stdint.h"
#include "lib/kernel/print.h"
#include "lib/kernel/io.h"
#include "kernel/global.h"

#define IDT_DESC_CNT 0x21

// 中断门描述符结构体
typedef struct _GateDesc {
    uint16 funcOffsetLowWord;
    uint16 selector;
    uint8 dCount;

    uint8 attribute;
    uint16 funcOffsetHighWord;
} GateDesc;
 

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

/* 
* 初始化可编程中断控制器
*/
static void PicInit(void) {
    // 初始化主片
    outb(PIC_M_CTRL, 0x11);
    outb(PIC_M_DATA, 0x20);
    
    outb(PIC_M_DATA, 0x04);
    outb(PIC_M_DATA, 0x01);

    // 初始化从片
    outb(PIC_S_CTRL, 0x11);
    outb(PIC_S_DATA, 0x28);

    outb(PIC_S_DATA, 0x02);
    outb(PIC_S_DATA, 0x01);


    // 打开主片上IR0，只支持时钟产生的中断
    outb(PIC_M_DATA, 0xfe);
    outb(PIC_S_DATA, 0xff);

    PutStr("    PicInit done\n");

}


static GateDesc gIdt[IDT_DESC_CNT];
extern IntrHandler gIntrEntryTable[IDT_DESC_CNT];
static void MakeIdtDesc(GateDesc* pGDesc, uint8 attr, IntrHandler function) {
    pGDesc->funcOffsetLowWord = (uint32)function & 0x0000ffff;
    pGDesc->selector = SELECTOR_K_CODE;
    pGDesc->dCount = 0;
    pGDesc->attribute = attr;
    pGDesc->funcOffsetHighWord = ((uint32)function & 0xffff0000) >> 16;
}

static void IdtDescInit(void) {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        MakeIdtDesc(&gIdt[i], IDT_DESC_ATTR_DPL0, gIntrEntryTable[i]);
    }
    PutStr("    IdtDescInitDone\n");
}

/* 
* 通用中断处理例程
* 从kernel.asm回调
*/
static void GeneralIntrHandler(uint8 vecNr) {
    if (vecNr == 0x27 || vecNr == 0x2f) {
        // IRQ7 和 IRQ15会产生伪中断，无需处理
        return;
    }
    PutStr("int vector : 0x"); PutInt(vecNr); PutChar('\n');
}

char* gIntrName[IDT_DESC_CNT];
IntrHandler gIdtTable[IDT_DESC_CNT];
extern IntrHandler gIntrEntryTable[IDT_DESC_CNT];
static void ExceptionInit(void) {
    for (int i = 0; i < IDT_DESC_CNT; i++) {
        gIdtTable[i] = GeneralIntrHandler;      // 默认为通用中断处理例程
        gIntrName[i] = "unknown";
    }
    gIntrName[0] = "#DE Divide Error";
    gIntrName[1] = "#DB Debug Exception";
    gIntrName[2] = "NMI Interrupt";
    gIntrName[3] = "#BP Breakpoint Exception";
    gIntrName[4] = "#OF Overflow Exception";
    gIntrName[5] = "#BR BOUND Range Exceeded Exception";
    gIntrName[6] = "#UD Invalid Opcode Exception";
    gIntrName[7] = "#NM Device Not Available Exception";
    gIntrName[8] = "#DF Double Fault Exception";
    gIntrName[9] = "Coprocessor Segment Overrun";
    gIntrName[10] = "#TS Invalid TSS Exception";
    gIntrName[11] = "#NP Segment Not Present";
    gIntrName[12] = "#SS Stack Fault Exception";
    gIntrName[13] = "#GP General Protection Exception";
    gIntrName[14] = "#PF Page-Fault Exception";
    // gIntrName[15] 第15项是intel保留项，未使用
    gIntrName[16] = "#MF x87 FPU Floating-Point Error";
    gIntrName[17] = "#AC Alignment Check Exception";
    gIntrName[18] = "#MC Machine-Check Exception";
    gIntrName[19] = "#XF SIMD Floating-Point Exception";
}

void IdtInit(void) {
    PutStr("IdtInit Start\n");
    IdtDescInit();      // 初始化中断描述符
    ExceptionInit();        // 初始化异常处理机制
    PicInit();      // 初始化8259A

    // 加载Idt
    uint64 idtOperand = ((sizeof(gIdt) - 1) | ((uint64)(uint32)gIdt << 16));
    asm volatile("lidt %0" : : "m"(idtOperand));
    PutStr("IdtInit done\n");
}


#define EFLAGS_IF 0x00000200
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g"(EFLAG_VAR))

/* 
* 开启中断
* 返回开启前的状态
*/
IntrStatus IntrEnable(void) {
    IntrStatus oldStatus;
    if (kIntrOn == IntrGetStatus()) {
        oldStatus = kIntrOn;
        return oldStatus;
    } else {
        oldStatus = kIntrOff;
        asm volatile("sti");
        return oldStatus;
    }
}

/* 
* 关闭中断
* 返回关闭前的状态
*/
IntrStatus IntrDisable(void) {
    IntrStatus oldStatus;
    if (kIntrOn == IntrGetStatus()) {
        oldStatus = kIntrOn;
        asm volatile("cli" : : : "memory");
        return oldStatus;
    } else {
        oldStatus = kIntrOff;
        return oldStatus;
    }
}

/* 
* 设置中断状态
* 返回原先的状态
*/
IntrStatus IntrSetStatus(IntrStatus status) {
    return status & kIntrOn ? IntrEnable() : IntrDisable();
}

/* 
* 获取中断状态
*/
IntrStatus IntrGetStatus(void) {
    uint32 eflags = 0;
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? kIntrOn : kIntrOff;
}