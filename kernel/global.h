#ifndef KERNEL_GLOBAL_H_
#define KERNEL_GLOBAL_H_

#include "lib/stdint.h"

/*
* GDT描述符属性
*/
#define DESC_G_4K (1 << 23)
#define DESC_D_32 (1 << 22)
#define DESC_L (0 << 21)
#define DESC_AVL (0 << 20)
#define DESC_P (1 << 15)
#define DESC_DPL_0 (0 << 13)
#define DESC_DPL_1 (1 << 13)
#define DESC_DPL_2 (2 << 13)
#define DESC_DPL_3 (3 << 13)

#define DESC_S_CODE (1 << 12)
#define DESC_S_DATA (1 << 12)
#define DESC_S_SYS (0 << 12)
#define DESC_TYPE_CODE (8 << 8)
#define DESC_TYPE_DATA (2 << 8)
#define DESC_TYPE_TSS (9 << 8)



#define RPL0 0
#define RPL1 1
#define RPL2 2
#define RPL3 3

#define TI_GDT (0 << 2)
#define TI_LDT (1 << 2)

#define SELECTOR_K_CODE ((1 << 3) | TI_GDT | RPL0)
#define SELECTOR_K_DATA ((2 << 3) | TI_GDT | RPL0)
#define SELECTOR_K_STACK SELECTOR_K_DATA
#define SELECTOR_K_GS ((3 << 3) | TI_GDT | RPL0)

#define SELECTOR_U_CODE ((5 << 3) | TI_GDT | RPL3)
#define SELECTOR_U_DATA ((6 << 3) | TI_GDT | RPL3)
#define SELECTOR_U_STACK SELECTOR_U_DATA


#define GDT_CODE_ATTR (DESC_G_4K | DESC_D_32 | DESC_L | DESC_AVL | DESC_P | DESC_DPL_3 | DESC_S_CODE | DESC_TYPE_CODE)
#define GDT_DATA_ATTR (DESC_G_4K | DESC_D_32 | DESC_L | DESC_AVL | DESC_P | DESC_DPL_3 | DESC_S_DATA | DESC_TYPE_DATA)


/*
* TSS描述符属性
*/
#define TSS_DESC_D (0 << 22)
#define TSS_ATTR (DESC_G_4K | TSS_DESC_D | DESC_L | DESC_AVL | DESC_P | DESC_DPL_0 | DESC_S_SYS | DESC_TYPE_TSS | 0)

#define SELECTOR_TSS ((4 << 3) | TI_GDT | RPL0)

typedef struct _GdtDesc {
    uint16_t limitLowWord;
    uint16_t baseLowWord;
    uint32_t highDword;
} GdtDesc;


#define IDT_DESC_P (1 << 7)
#define IDT_DESC_DPL0 (0 << 5)
#define IDT_DESC_DPL3 (3 << 5)
#define IDT_DESC_32_TYPE 0xe
#define IDT_DESC_16_TYPE 0x6

#define IDT_DESC_ATTR_DPL0 (IDT_DESC_P | IDT_DESC_DPL0 | IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 (IDT_DESC_P | IDT_DESC_DPL3 | IDT_DESC_32_TYPE)



/*
* EFL寄存器属性
*/
#define EFLAGS_MBS (1 << 1)
#define EFLAGS_IF_1 (1 << 9)
#define EFLAGS_IF_0 (0 << 9)
#define EFLAGS_IOPL_3 (3 << 12)

#define EFLAGS_IOPL_0 (0 << 12)

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))
#endif // KERNEL_GLOBAL_H_