#ifndef DEVICE_CONSOLE_H_
#define DEVICE_CONSOLE_H_

#include "console.h"

#include "lib/stdint.h"
#include "lib/kernel/print.h"
#include "thread/sync.h"
#include "thread/thread.h"

static Lock gsConsoleLock;

void ConsoleInit(void);
void ConsoleAcquire(void);
void ConsoleRelease(void);
void ConsolePutStr(char* str);
void ConsolePutChar(uint8_t charAscii);
void ConsolePutInt(uint32_t num);

#endif // DEVICE_CONSOLE_H_