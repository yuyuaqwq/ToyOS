#include "stdio-kernel.h"

#include "lib/stdio.h"
#include "device/console.h"

void PrintK(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    va_end(args);
    ConsolePutStr(buf);
}