#ifndef KERNEL_INTERRUPT_H_
#define KERNEL_INTERRUPT_H_

#include "lib/stdint.h"

typedef void* IntrHandler;
void IdtInit(void);

/* 
* 中断状态
*/
typedef enum _IntrStatus {
    kIntrOff,       // 关闭
    kIntrOn,        // 开启
} IntrStatus;
IntrStatus IntrGetStatus(void);
IntrStatus IntrSetStatus(IntrStatus status);
IntrStatus IntrEnable(void);
IntrStatus IntrDisable(void);

void IntrRegisterHandler(uint8 vectorNo, IntrHandler function);

#endif // KERNEL_INTERRUPT_H_