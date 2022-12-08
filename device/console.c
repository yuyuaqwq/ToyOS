#include "console.h"

#include "lib/stdint.h"
#include "lib/kernel/print.h"
#include "thread/sync.h"
#include "thread/thread.h"

static Lock gsConsoleLock;

void ConsoleInit(void) {
    LockInit(&gsConsoleLock);
}

void ConsoleAcquire(void) {
    LockAcquire(&gsConsoleLock);
}

void ConsoleRelease(void) {
    LockRelease(&gsConsoleLock);
}

void ConsolePutStr(char* str) {
    ConsoleAcquire();
    PutStr(str);
    ConsoleRelease();
}

void ConsolePutChar(uint8 charAscii) {
    ConsoleAcquire();
    PutChar(charAscii);
    ConsoleRelease();
}

void ConsolePutInt(uint32 num) {
    ConsoleAcquire();
    PutInt(num);
    ConsoleRelease();
}