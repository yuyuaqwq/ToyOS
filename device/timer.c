#include "timer.h"

#include "kernel/debug.h"
#include "kernel/interrupt.h"
#include "thread/thread.h"


#include "lib/kernel/io.h"
#include "lib/kernel/print.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define COUNTER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

static void FrequencySet(uint8_t counterPort, uint8_t counterNo, uint8_t rwl, uint8_t counterMode, uint16_t counterValue) {
    outb(PIT_CONTROL_PORT, (uint8_t )((counterNo << 6) | (rwl << 4) | (counterMode << 1)));
    outb(counterPort, (uint8_t )counterValue);
    outb(counterPort, (uint8_t )(counterValue >> 8));
}

#define MILLI_SECONDS_PER_INTR (1000 / IRQ0_FREQUENCY)

uint32_t gTicks;
static void IntrTimerHandler(void) {
    TaskStruct* curThread = RunningThread();
    ASSERT(curThread->stackMagic == 'THRE');

    curThread->elapsedTicks++;
    gTicks++;

    if (curThread->ticks == 0) {
        Schedule();
    } else {
        curThread->ticks--;
    }
}

static void TicksToSleep(uint32_t sleepTicks) {
    uint32_t startTick = gTicks;

    while (gTicks - startTick < sleepTicks) {
        ThreadYIeld();
    }
}

void MilliTimeSleep(uint32_t milliSeconds) {
    uint32_t sleepTicks = DIV_ROUND_UP(milliSeconds, MILLI_SECONDS_PER_INTR);
    ASSERT(sleepTicks > 0);
    TicksToSleep(sleepTicks);
}

void TimerInit(void) {
    PutStr("TimerInit start\n");
    FrequencySet(COUNTER0_PORT, COUNTER0_NO, READ_WRITE_LATCH, COUNTER_MODE, COUNTER0_VALUE);
    IntrRegisterHandler(0x20, IntrTimerHandler);
    PutStr("TimerInit done\n");
}