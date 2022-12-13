#include "stdio.h"

#include "lib/stdint.h"
#include "lib/user/syscall.h"


#define va_start(ap, v) ap = (va_list)&v
#define va_arg(ap, t) *((t*)(ap += 4))
#define va_end(ap) ap = NULL

static void itoa(uint32_t value, char** bufPtrAddr, uint32_t base) {
    uint32_t m = value % base;
    uint32_t i = value / base;
    if (i) {
        itoa(i, bufPtrAddr, base);
    }
    if (m < 10) {
        *((*bufPtrAddr)++) = m + '0';
    } else {
        *((*bufPtrAddr)++) = m - 10 + 'A';
    }
}

uint32_t vsprintf(char* str, const char* format, va_list ap) {
    char* bufPtr = str;
    const char* indexPtr = format;
    char indexChar = *indexPtr;
    int32_t argInt;
    char* argStr;
    while (indexChar) {
        if (indexChar != '%') {
            *(bufPtr++) = indexChar;
            indexChar = *(++indexPtr);
            continue;
        }
        indexChar = *(++indexPtr);
        switch(indexChar) {
            case 's': {
                argStr = va_arg(ap, char*);
                strcpy(bufPtr, argStr);
                bufPtr += strlen(argStr);
                indexChar = *(++indexPtr);
                break;
            }
            case 'c': {
                *(bufPtr) = va_arg(ap, int);
                indexChar = *(++indexPtr);
                break;
            }
            case 'd': {
                argInt = va_arg(ap, int);
                if (argInt < 0) {
                    argInt = 0 - argInt;
                    *bufPtr++ = '-';
                }
                itoa(argInt, &bufPtr, 10);
                indexChar = *(++indexPtr);
                break;
            }
            case 'x': {
                argInt = va_arg(ap, int);
                itoa(argInt, &bufPtr, 16);
                indexChar = *(++indexPtr);
                break;
            }
        }
    }
    return strlen(str);
}

uint32_t sprintf(char* buf, const char* format, ...) {
    va_list args;
    uint32_t retval;
    va_start(args, format);
    retval = vsprintf(buf, format, args);
    va_end(args);
    return retval;
}

uint32_t printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buf[1024] = {0};
    vsprintf(buf, format, args);
    va_end(args);
    return Write(buf);
}