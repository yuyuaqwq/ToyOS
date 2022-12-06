#include "init.h"

#include "lib/kernel/print.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "device/timer.h"
#include "thread/thread.h"

void InitAll(void) {
    PutStr("InitAll\n");
    IdtInit();
    MemInit();
    ThreadEnvirInit();
    TimerInit();
}