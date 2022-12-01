#include "init.h"

#include "lib/kernel/print.h"
#include "kernel/interrupt.h"
#include "device/timer.h"

void InitAll(void) {
    PutStr("InitAll\n");
    IdtInit();
    TimerInit();
}