#ifndef KERNEL_GLOBAL_H_
#define KERNEL_GLOBAL_H_

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


#define IDT_DESC_P (1 << 7)
#define IDT_DESC_DPL0 (0 << 5)
#define IDT_DESC_DPL3 (3 << 5)
#define IDT_DESC_32_TYPE 0xe
#define IDT_DESC_16_TYPE 0x6

#define IDT_DESC_ATTR_DPL0 (IDT_DESC_P | IDT_DESC_DPL0 | IDT_DESC_32_TYPE)
#define IDT_DESC_ATTR_DPL3 (IDT_DESC_P | IDT_DESC_DPL3 | IDT_DESC_32_TYPE)

#endif // KERNEL_GLOBAL_H_