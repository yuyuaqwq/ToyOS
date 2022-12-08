#ifndef DEVICE_KEYBOARD_H_
#define DEVICE_KEYBOARD_H_

#include "device/ioqueue.h"

extern IoQueue gKbdBuf;

void KeyBoardInit(void);

#endif // DEVICE_KEYBOARD_H_