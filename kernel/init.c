#include "init.h"

#include "lib/kernel/print.h"
#include "kernel/interrupt.h"
#include "kernel/memory.h"
#include "device/timer.h"
#include "device/console.h"
#include "device/keyboard.h"
#include "thread/thread.h"
#include "userprog/tss.h"

void InitAll(void) {
    PutStr("InitAll\n");
    IdtInit();
    MemInit();
    ThreadEnvirInit();
    TimerInit();
    KeyBoardInit();
    ConsoleInit();
    TssInit();
}