#ifndef DEVICE_TIMER_H_
#define DEVICE_TIMER_H_

#include "lib/stdint.h"

void TimerInit(void);

void MilliTimeSleep(uint32_t milliSeconds);

#endif // DEVICE_TIMER_H_